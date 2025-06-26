#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <stdexcept>
#include <unistd.h>
#include <dirent.h>
#include <cstdio>
#include "../../include/process/ProcessRequest.hpp"
#include "../../include/http/HttpErrorException.hpp"
#include "../../include/http/RequestParser.hpp"
#include "../../include/http/HttpUtils.hpp"
#include "../../include/http/HttpRequest.hpp"
#include "../../include/http/HttpResponse.hpp"
#include "../../include/config/Location.hpp"
#include "../../include/config/ServerConfig.hpp"
#include "../../include/process/CGIHandler.hpp"

static void checkDeleteValidity(const std::string & path);
static std::string createIndexPath(std::string path, const Location & location);
static std::string generateAutoIndex(const std::string & dirPath, const std::string & uriPath);
static std::string sanitizeFilenamePart(const std::string & input);
static std::string createUploadFilename(const std::string & path);
static bool isCgi(const std::string & path);
static std::string findBoundary(const std::string & contentType);

/* ************************************************************************** */
/*                                  constructors                              */
/* ************************************************************************** */

ProcessRequest::ProcessRequest() :
	_serversVector(),
	_processStatus(WAITING_HEADERS),
	_server(),
	_serverTimeout(0),
	_location(),
	_inputData(""),
	_outputData(""),
	_httpResponse(),
	_bytesSent(0),
	_handler(NULL),
	_request(NULL),
	_file(NULL)
{}

ProcessRequest::ProcessRequest(const std::vector<ServerConfig> & serversVector) :
	_serversVector(serversVector),
	_processStatus(WAITING_HEADERS),
	_location(),
	_inputData(""),
	_outputData(""),
	_httpResponse(),
	_bytesSent(0),
	_handler(NULL),
	_request(NULL),
	_file(NULL)
{
	if (serversVector.empty())
		throw HttpErrorException(500, "in PR: ServerConfig is empty.");
	_server = serversVector[0];
	_serverTimeout = _server.getSessionTimeout();
}

ProcessRequest::ProcessRequest(const ProcessRequest & other) :
	_serversVector(other._serversVector),
	_processStatus(other._processStatus),
	_server(other._server),
	_serverTimeout(other._serverTimeout),
	_location(other._location),
	_inputData(other._inputData),
	_outputData(other._outputData),
	_httpResponse(other._httpResponse),
	_bytesSent(other._bytesSent),
	_handler(other._handler)
{
	if (!other._file)
		_file = NULL;
	else {
		try {
			_file = new File(*other._file);
		}
		catch (const std::bad_alloc&) {
			throw HttpErrorException(500, "in PR: Bad alloc.");
		}
	}
	if (!other._request)
		_request = NULL;
	else {
		try {
			_request = new HttpRequest(*other._request);
		}
		catch (const std::bad_alloc&) {
			throw HttpErrorException(500, "in PR: Bad alloc.");
		}
	}
}

/* ************************************************************************** */
/*                                    operators                               */
/* ************************************************************************** */

ProcessRequest & ProcessRequest::operator=(const ProcessRequest & other)
{
	if (&other != this) {
		_serversVector = other._serversVector;
		_processStatus = other._processStatus;
		_server = other._server;
		_serverTimeout = other._serverTimeout;
		_location = other._location;
		_serverTimeout = other._serverTimeout;
		_handler = other._handler;
		_httpResponse = other._httpResponse;
		_inputData = other._inputData;
		_outputData = other._outputData;
		_bytesSent = other._bytesSent;
		delete _file;
		delete _request;
		if (!other._file)
			_file = NULL;
		else {
			try {
				_file = new File(*other._file);
			}
			catch (const std::bad_alloc&) {
				throw HttpErrorException(500, "in PR: Bad alloc.");
			}
		}
		if (!other._request)
			_request = NULL;
		else {
			try {
				_request = new HttpRequest(*other._request);
			}
			catch (const std::bad_alloc&) {
				throw HttpErrorException(500, "in PR: Bad alloc.");
			}
		}
	}
	return (*this);
}

/* ************************************************************************** */
/*                                   destructor                               */
/* ************************************************************************** */

ProcessRequest::~ProcessRequest()
{
	delete _file;
	delete _request;
}

/* ************************************************************************** */
/*                                     process                                */
/* ************************************************************************** */

std::string ProcessRequest::process(std::string data)
{
	if (_processStatus == WAITING_HEADERS && _inputData.size() + data.size() > 80192)
		throw HttpErrorException(431, "in PR: Input data too large for headers.");

	_inputData.append(data);

	switch (_processStatus) {
		case WAITING_HEADERS:
			waitHeaders();
			break ;
		case HANDLING_METHOD:
			handleMethod();
			break ;
		case WAITING_BODY:
			waitBody();
			break ;
		case SENDING_HEADERS:
			sendHeaders();
			break ;
		case SENDING_BODY:
			sendBody();
			break ;
		case DONE:
			if (!_inputData.empty())
				_inputData.clear();
			if (!_outputData.empty())
				_outputData.clear();
			break;
		default:
			throw HttpErrorException(500, "in PR: Invalid default switch case.");
	}
	std::string dataToSend = _outputData;
	_outputData.clear();	
	return (dataToSend);
}

// From status WAITING_HEADERS to HANDLING_METHOD
void ProcessRequest::waitHeaders()
{
	if (_processStatus != WAITING_HEADERS)
		return ;

	size_t pos = _inputData.find("\r\n\r\n");
	if (pos != std::string::npos) {
		std::string headersPart = _inputData.substr(0, pos + 4);
		std::string bodyPart = _inputData.substr(pos + 4);
		RequestParser parser(headersPart);
		_request = parser.release();
		_inputData = bodyPart;
		selectServer();
		_serverTimeout = _server.getSessionTimeout();
		selectLocation();
		if (_request->getContentLength() > selectMaxBodySize())
			throw HttpErrorException(413, "in PR: Content-Length too large.");
		_processStatus = HANDLING_METHOD;
		handleMethod();
	}
}
// From status HANDLING_METHOD to WAITING_BODY or SENDING_REQUEST
void ProcessRequest::handleMethod()
{
	if (_processStatus != HANDLING_METHOD)
		return ;

	selectHandler();
	if (_location.hasRedirect()) {
		ResponseBuilder::buildRedirect(this);
		_processStatus = SENDING_HEADERS;
		sendHeaders();
	}
	if (!_handler)
		throw HttpErrorException(500, "in PR: Unable to select handler.");
	(this->*_handler)();
}

// From status WAITING_BODY to SENDING_HEADERS
void ProcessRequest::waitBody()
{
	if (_processStatus  != WAITING_BODY)
		return ;
	if (!_request)
		throw HttpErrorException(500, "in PR: Request is NULL.");

	if (_file)
		writeBodyWithFile();
	else
		writeBodyWithoutFile();
}

void ProcessRequest::writeBodyWithFile()
{
	_bytesSent += _inputData.size();
	size_t remainingBytes = _request->getContentLength() - _file->getOffset();
	size_t bytesToWrite = (_inputData.size() > remainingBytes) ? remainingBytes : _inputData.size();

	if (bytesToWrite) {
		if (_file->WriteChunk(_inputData.c_str(), bytesToWrite) != bytesToWrite)
			throw HttpErrorException(500, "in PR: bytes written not equal to bytes to write.");
	}
	_inputData.clear();
	if (_bytesSent >= _request->getContentLength()) {
		_file->closeFile();

		std::string originalPath = _file->getPath();
		std::string headerName   = _file->getName();
		std::string newPath = originalPath;
		if (!headerName.empty()) {
			headerName = sanitizeFilenamePart(headerName);
			newPath = originalPath + "-" + headerName;
			rename(originalPath.c_str(), newPath.c_str());
		}
		std::string relativeFilePath = createRelativeFilePath(newPath);
		std::map<std::string, std::string> headers;
		headers["location"] = relativeFilePath;
		headers["content-type"] = "text/html";
		std::string body =
			"<html><body><h1>201 Created</h1>\n"
			"<p>The resource has been successfully created.</p>\n"
			"<a href=\"" + relativeFilePath  + "\">See file</a>\n"
			"<a href=\"/\">Home</a>\n" 
			"</body></html>";
		headers["content-length"] = HttpUtils::numberToString(body.length());
		ResponseBuilder::buildResponse(this, 201, headers, body);
		delete _file;
		_file = NULL;
		_processStatus = SENDING_HEADERS;
		sendHeaders();
	}
}

void ProcessRequest::writeBodyWithoutFile()
{
	size_t remainingBytes = _request->getContentLength() - _request->getBody().size();

	std::string dataToAppend;
	if (_inputData.size() <= remainingBytes)
		dataToAppend = _inputData;
	else
		dataToAppend = _inputData.substr(0, remainingBytes);

	_request->AppendBody(dataToAppend);
	_inputData.clear();

	if (_request->getBody().size() >= _request->getContentLength()) {
		if (isCgi(createPath())) {
				std::string path = createPath();
				CGIHandler cgi(*_request, path);
				_httpResponse = cgi.getHttpResponse();
				ResponseBuilder::addFinalHeaders(this);
				_processStatus = SENDING_HEADERS;
				sendHeaders();
				return ;
		}
		_processStatus = SENDING_HEADERS;
		sendHeaders();
	}
}

// From status SENDING_HEADERS to SENDING_BODY or DONE
void ProcessRequest::sendHeaders()
{
	if (_processStatus != SENDING_HEADERS)
		return ;
	
	_outputData = _httpResponse.toRawString();
	if (_file != NULL)
		_processStatus = SENDING_BODY;
	else
		_processStatus = DONE;
}

// From status SENDING_BODY to DONE
void ProcessRequest::sendBody()
{
	if (_processStatus != SENDING_BODY)
		return ;
	if (!_file)
		throw HttpErrorException(500, "in PR: File is NULL.");

	char buffer[BUFFER_SIZE];
	memset(buffer, 0, BUFFER_SIZE);

	size_t result = _file->ReadChunk(buffer, BUFFER_SIZE);
	if (result == 0 || _file->getOffset() >= static_cast<size_t>(_file->getSize())) {
		_file->closeFile();
		_processStatus = DONE;
	}
	_outputData = std::string(buffer, result);
}

/* ************************************************************************** */
/*                                  error builder                             */
/* ************************************************************************** */

void ProcessRequest::errorBuilder(int statusCode, bool secondTime)
{
	ResponseBuilder::errorBuilder(this, statusCode, secondTime);
	_processStatus = SENDING_HEADERS;
}

/* ************************************************************************** */
/*                                      handlers                              */
/* ************************************************************************** */

void ProcessRequest::deleteHandler()
{
	if (!_request)
		throw HttpErrorException(500, "in PR: Request is NULL.");
	if (_processStatus != HANDLING_METHOD)
		return ;

	std::string path = createPath();
	checkDeleteValidity(path);

	if (unlink(path.c_str()) == -1)
		throw HttpErrorException(500, "in PR: Unlink failed.");

	std::map<std::string, std::string> headers;
	headers["content-length"] = "0";
	ResponseBuilder::buildResponse(this, 204, headers, "");
	_processStatus = SENDING_HEADERS;
	sendHeaders();
}

void ProcessRequest::getHandler()
{
	if (!_request)
		throw HttpErrorException(500, "in PR: Request is NULL.");
	if (_processStatus != HANDLING_METHOD)
		return ;
	
	std::string path = createPath();
	if (!HttpUtils::fileExists(path))
		throw HttpErrorException(404, "in PR: Invalid path.");

	if (HttpUtils::isDirectory(path)) {
		std::string indexFile = createIndexPath(path, _location);
		if (HttpUtils::fileExists(indexFile))
			path = indexFile;
		else if (_location.isAutoIndex()) {
			std::string body = generateAutoIndex(path, _request->getTarget());
			std::map<std::string, std::string> headers;
			headers["content-type"] = "text/html";
			headers["content-length"] = HttpUtils::numberToString(body.length());
			ResponseBuilder::buildResponse(this, 200, headers, body);
			_processStatus = SENDING_HEADERS;
			sendHeaders();
			return ;
		}
		else
			throw HttpErrorException(403, "in PR: Invalid directory path.");
	}
	delete _file;
	_file = NULL;
	try {
		_file = new File(path);
	}
	catch (const std::bad_alloc&) {
		throw HttpErrorException(500, "in PR: Bad alloc.");
	}
	std::map<std::string, std::string> headers;
	headers["content-type"] = _file->getMimeType();
	headers["content-length"] = HttpUtils::numberToString(_file->getSize());
	ResponseBuilder::buildResponse(this, 200, headers, "");

	_processStatus = SENDING_HEADERS;
	sendHeaders();
}

void ProcessRequest::postHandler()
{
	if (_processStatus != HANDLING_METHOD)
		return ;
	if (!_request)
		throw HttpErrorException(500, "in PR: Request is NULL.");
	if (_file)
		throw HttpErrorException(500, "in PR: File exists but should not.");

	std::string contentType = "";
	if (_request->hasHeader("content-type"))
		contentType = _request->getHeaderValue("content-type");

	if (!contentType.empty() && (
			(contentType.find("multipart/form-data") == 0 &&
			contentType.find("boundary=") != std::string::npos) ||
			contentType.find("application/octet-stream") == 0))
		handleUpload(contentType);
	else {
		std::string path = createPath();
		checkPostValidity(path);
	}
	_processStatus = WAITING_BODY;
	waitBody();
}

void ProcessRequest::handleUpload(const std::string & contentType)
{
	std::string path = createUploadPath();
	checkPostValidity(path);
	std::string filepath = createUploadFilename(path);

	if (contentType.find("application/octet-stream") == 0) {
		try {
			_file = new File(filepath, true);
		}
		catch (const std::bad_alloc&) {
			throw HttpErrorException(500, "in PR: Bad alloc.");
		}
	}
	else {
		std::string boundary = findBoundary(contentType);
		if (boundary.empty())
			throw HttpErrorException(400, "in PR: No boundary.");
		try {
			_file = new File(filepath, boundary);
		}
		catch (const std::bad_alloc&) {
			throw HttpErrorException(500, "in PR: Bad alloc.");
		}
	}
}

void ProcessRequest::cgiGetHandler()
{
	std::string path = createPath();

	CGIHandler cgi(*_request, path);
	_httpResponse = cgi.getHttpResponse();
	ResponseBuilder::addFinalHeaders(this);

	_processStatus = SENDING_HEADERS;
	sendHeaders();
}

/* ************************************************************************** */
/*                                   selectors                                */
/* ************************************************************************** */

void ProcessRequest::selectServer()
{
	if (!_request)
		throw HttpErrorException(500, "in PR: Request is NULL.");

	const std::map<std::string, std::string> & headers = _request->getHeaders();
	std::map<std::string, std::string>::const_iterator headersCit = headers.find("host"); 
	if (headersCit != headers.end()) {
		std::vector<ServerConfig>::const_iterator serverCit = _serversVector.begin();
		for (; serverCit != _serversVector.end(); ++serverCit) {
			if (serverCit->hasServerName(headersCit->second)) {
				_server = *serverCit;
				return ;
			}
		}
	}
	if (_serversVector.empty()) {
		throw HttpErrorException(500, "in PR: Unable to select server.");
	}
	_server = _serversVector[0];
}

void ProcessRequest::selectLocation()
{
	if (!_request)
		throw HttpErrorException(500, "in PR: Request is NULL.");

	const std::map<std::string, Location> & locations = _server.getLocations();

	const std::string & target = _request->getTarget();

	std::string bestMatch = "";
	std::map<std::string, Location>::const_iterator it;
	for (it = locations.begin(); it != locations.end(); ++it)
	{
		const std::string & path = it->first;
		if (target.compare(0, path.size(), path) == 0)
		{
			if (path.size() > bestMatch.size())
				bestMatch = path;
		}
	}
	if (bestMatch.empty())
		throw HttpErrorException(404, "in PR: Unable to select location.");
	
	_location = locations.find(bestMatch)->second;
}

void ProcessRequest::selectHandler()
{
	if (!_request)
		throw HttpErrorException(500, "in PR: Request is NULL.");

	const std::string & method = _request->getMethod();
	if (isCgi(_request->getTarget()) && method == "GET")
		_handler = &ProcessRequest::cgiGetHandler;
	else if (method == "DELETE")
		_handler = &ProcessRequest::deleteHandler;
	else if (method == "POST")
		_handler = &ProcessRequest::postHandler;
	else if (method == "GET")
		_handler = &ProcessRequest::getHandler;
	else
		throw HttpErrorException(501, "in PR: Method not implemented.");
	if (!_location.isValidMethod(method))
		throw HttpErrorException(405, "in PR: Method not allowed.");
}

const std::string & ProcessRequest::selectRoot()
{
	return (!_location.getRoot().empty() ? _location.getRoot() : _server.getRoot());
}

size_t ProcessRequest::selectMaxBodySize()
{
	if (_location.getClientMaxBodySize() >= 0)
		return (static_cast<size_t>(_location.getClientMaxBodySize()));
	return (static_cast<size_t>(_server.getClientMaxBodySize()));
}

/* ************************************************************************** */
/*                                   getters                                  */
/* ************************************************************************** */

ProcessStatus ProcessRequest::getProcessStatus() const
{
	return (_processStatus);
}

const ServerConfig & ProcessRequest::getServer() const
{
	return (_server);
}

File * ProcessRequest::getFilePtr() const
{
	if (_file)
		return (_file);
	return (NULL);
}

/* ************************************************************************** */
/*                            private utils methods                           */
/* ************************************************************************** */

std::string ProcessRequest::createPath()
{
	if (!_request)
		throw HttpErrorException(500, "in PR: Request is NULL.");

	std::string locationPath = _location.getPath();
	std::string target = _request->getTarget();
	std::string root = selectRoot();

	if (target.find(locationPath) != 0)
		throw HttpErrorException(404, "in PR: Invalid path.");

	std::string relativePath = target.substr(locationPath.size());
	HttpUtils::trimFinalSlash(root);
	HttpUtils::trimSlashes(relativePath);

	return (root + '/' + relativePath);
}

std::string ProcessRequest::createUploadPath()
{
	if (!_request)
		throw HttpErrorException(500, "in PR: Request is NULL.");

	std::string root = selectRoot();
	std::string locationPath = _location.getUploadPath();
	HttpUtils::trimFinalSlash(root);
	HttpUtils::trimSlashes(locationPath);

	return (root + '/' + locationPath);
}

std::string ProcessRequest::createUploadPathForClient()
{
	if (!_request)
		throw HttpErrorException(500, "in PR: Request is NULL.");

	std::string path = _location.getPath();
	std::string locationPath = _location.getUploadPath();
	HttpUtils::trimFinalSlash(path);
	HttpUtils::trimSlashes(locationPath);

	return (path + '/' + locationPath);
}

std::string ProcessRequest::createRelativeFilePath(const std::string & realFilePath)
{
	std::string uploadPath = createUploadPath();
	std::string clientPath = createUploadPathForClient();
	std::string relativeFilePath = clientPath + realFilePath.substr(uploadPath.size());
	return (relativeFilePath);
}

void ProcessRequest::checkPostValidity(const std::string & path)
{
	if (_file) {
		if (_location.getUploadPath().empty())
			throw HttpErrorException(500, "in PR: No upload path.");
		if (!HttpUtils::isDirectory(path))
			throw HttpErrorException(500, "in PR: Upload path is not a directory.");
		if (!HttpUtils::hasWritePermission(path))
			throw HttpErrorException(403, "in PR: Invalid permission for upload path.");
	}
	else {
		if (!HttpUtils::fileExists(path))
			throw HttpErrorException(404, "in PR: Unknown path.");
	}
}

/* ************************************************************************** */
/*                            non member functions                            */
/* ************************************************************************** */

static void checkDeleteValidity(const std::string & path)
{
	if (!HttpUtils::fileExists(path))
		throw HttpErrorException(404, "in PR: Unknown path.");
	if (!HttpUtils::isRegularFile(path))
		throw HttpErrorException(403, "in PR: Path is not a regular file.");
	if (!HttpUtils::hasWritePermission(path))
		throw HttpErrorException(403, "in PR: Invalid permission for delete.");
}

static std::string createIndexPath(std::string path, const Location & location)
{
	if (!path.empty() && path[path.size() - 1] == '/') {
		path.erase(path.size() - 1); }
	std::string indexFile = location.getIndex();
	if (indexFile.empty()) {
		indexFile = "/index.html"; }
	return (path + '/' + indexFile);
}

static std::string generateAutoIndex(const std::string & dirPath, const std::string & uriPath)
{
	DIR* dir = opendir(dirPath.c_str());
	if (!dir)
		throw HttpErrorException(500, "in PR: Opendir failed.");
	std::ostringstream html;
	html << "<html><body><h1>Index of " << uriPath << "</h1><ul>";
	struct dirent* entry;
	while ((entry = readdir(dir)) != NULL) {
		std::string name = entry->d_name;
		if (name == "." || name == "..") {
			continue ; }
		html << "<li><a href=\"" << uriPath;
		if (!uriPath.empty() && uriPath[uriPath.size() - 1] != '/')
			html << "/";
		html << name << "\">" << name << "</a></li>"; }
	closedir(dir);
	html << "</ul>\n"
		<< "<footer><a href=\"/\">Home</a></footer>\n"
		<< "</body></html>";
	return (html.str());
}

static std::string sanitizeFilenamePart(const std::string & input)
{
	std::string result;
	for (size_t i = 0; i < input.length(); ++i) {
		char c = input[i];
		if (std::isalnum(c) || c == '-' || c == '_' || c == '.')
			result += c;
		}
	return (result);
}

static std::string createUploadFilename(const std::string & path)
{
	return (path + "/" + HttpUtils::generateUniqueTimestamp());
}

static bool isCgi(const std::string & path)
{
	size_t start = path.find_last_of('.');
	if (start == std::string::npos)
		return (false);
	size_t end = path.find('?', start);
	std::string ext = path.substr(start, end - start);
	return (ext == ".py" || ext == ".php" || ext == ".pl");
}

static std::string findBoundary(const std::string & contentType)
{
	std::string boundary;

	size_t start = contentType.find("boundary=");
	if (start == std::string::npos)
		throw HttpErrorException(400, "in PR: No boundary found.");
	start += 9;

	size_t end = contentType.find(";", start);
	if (end == std::string::npos)
		boundary = contentType.substr(start);
	else
		boundary = contentType.substr(start, end - start);

	if (!boundary.empty() && boundary[0] == '"' && boundary[boundary.size() - 1] == '"')
		boundary = boundary.substr(1, boundary.size() - 2);

	return ("--" + boundary += "\r\n");
}
