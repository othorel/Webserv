#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <stdexcept>
#include <unistd.h>
#include <dirent.h>
#include "../../include/http/ProcessRequest.hpp"
# include "../../include/http/HttpErrorException.hpp"
# include "../../include/http/RequestParser.hpp"
# include "../../include/http/HttpUtils.hpp"
#include "../../include/http/HttpRequest.hpp"
#include "../../include/http/HttpResponse.hpp"
#include "../../include/config/Location.hpp"
#include "../../include/config/ServerConfig.hpp"

static void checkDeleteValidity(const std::string & path);
static std::string createIndexPath(std::string path, const Location & location);
static std::string generateAutoIndex(const std::string & dirPath, const std::string & uriPath);
static std::string sanitizeFilenamePart(const std::string & input);
static void checkPostValidity(
	const HttpRequest & request, const Location & location ,
	const ServerConfig & server, const std::string & path);
static std::string createPostFileName(
	const HttpRequest & request, const std::string & path);
// static std::string createPath(const std::string & root, const std::string & subpath);

/* ************************************************************************** */
/*                                  constructors                              */
/* ************************************************************************** */

ProcessRequest::ProcessRequest() :
	_serversVector(),
	_processStatus(WAITING_HEADERS),
	_server(),
	_location(),
	_inputData(""),
	_outputData(""),
	_httpResponse(),
	_handler(NULL),
	_request(NULL),
	_file(NULL)
{
	std::cout << "Default Creation of ProcessRequest" << std::endl;
}

ProcessRequest::ProcessRequest(const std::vector<ServerConfig> & serversVector) :
	_serversVector(serversVector),
	_processStatus(WAITING_HEADERS),
	_server(),
	_location(),
	_inputData(""),
	_outputData(""),
	_httpResponse(),
	_handler(NULL),
	_request(NULL),
	_file(NULL)
{
	std::cout << "Creation of ProcessRequest" << std::endl;
}

ProcessRequest::ProcessRequest(const ProcessRequest & other) :
	_serversVector(other._serversVector),
	_processStatus(other._processStatus),
	_server(other._server),
	_location(other._location),
	_inputData(other._inputData),
	_outputData(other._outputData),
	_httpResponse(other._httpResponse),
	_handler(other._handler)
{
	_file = other._file ? new File(*other._file) : NULL;
	_request = other._request ? new HttpRequest(*other._request) : NULL;
	std::cout << "Copy Creation of ProcessRequest" << std::endl;
}

void ProcessRequest::reset()
{
	if (_file) {
		delete _file;
		_file = NULL;
	}
	if (_request) {
		delete _request;
		_request = NULL;
	}
	_processStatus = WAITING_HEADERS;
	_server = ServerConfig();
	_location = Location();
	_handler = NULL;
	_httpResponse = HttpResponse();
	_inputData.clear();
	_outputData.clear();
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
		_location = other._location;
		_handler = other._handler;
		_httpResponse = other._httpResponse;
		_inputData = other._inputData;
		_outputData = other._outputData;
		if (_file)
			delete _file;
		_file = other._file ? new File(*other._file) : NULL;
		if (_request)
			delete _request;
		_request = other._request ? new HttpRequest(*other._request) : NULL;
	}
	std::cout << "Operator = of ProcessRequest" << std::endl;
	return (*this);
}

/* ************************************************************************** */
/*                                   destructor                               */
/* ************************************************************************** */

ProcessRequest::~ProcessRequest()
{
	if (_file)
		delete _file;
	if (_request)
		delete _request;
	std::cout << "Destruction of ProcessRequest" << std::endl;
}

/* ************************************************************************** */
/*                                     process                                */
/* ************************************************************************** */

std::string ProcessRequest::process(std::string data)
{
	_inputData.append(data);
	switch (_processStatus)
		{
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
				throw HttpErrorException(500);
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
		_request->debug();
		_inputData = bodyPart;
		selectServer();
		selectLocation();
		checkMethodValidity();
		if (_location.hasRedirect()) 
			buildRedirect();
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
	if (!_handler)
		throw HttpErrorException(500);
	(this->*_handler)();
}

// From status WAITING_BODY to SENDING_HEADERS
void ProcessRequest::waitBody()
{
	if (_processStatus  != WAITING_BODY)
		return ;

	if (!_request || !_file)
		throw HttpErrorException(500);

	size_t remainingBytes = _request->getContentLength() - _file->getOffset();
	size_t bytesToWrite = (_inputData.size() > remainingBytes) ? remainingBytes : _inputData.size();
	if (bytesToWrite) {
		if (_file->WriteChunk(_inputData.c_str(), bytesToWrite) != bytesToWrite)
			throw HttpErrorException(500);
	}
	_inputData.clear();

	if (_file->getOffset() >= _request->getContentLength()) {
		std::map<std::string, std::string> headers;
		std::string relativePath = _location.getUploadPath();
		if (!relativePath.empty() && relativePath[relativePath.size() - 1] != '/') {
			relativePath += "/"; }
		relativePath += _file->getPath().substr(_file->getPath().find_last_of("/") + 1);
		headers["location"] = relativePath;
		headers["content-type"] = "text/html";
		std::string body =
			"<html><body><h1>201 Created</h1>\n"
			"<p>The resource has been successfully created.</p>\n"
			"<a href=\"" + relativePath  + "\">See file</a>\n"
			"</body></html>";
		headers["content-length"] = HttpUtils::numberToString(body.length());
		buildResponse(201, headers, body);
		if (_file) {
			delete _file;
			_file = NULL;
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
	{
		_processStatus = SENDING_BODY;
	}
	else
		_processStatus = DONE;
}

// From status SENDING_BODY to DONE
void ProcessRequest::sendBody()
{
	if (_processStatus != SENDING_BODY)
		return ;
		
	if (!_file)
		throw HttpErrorException(500);
	
	char buffer[BUFFER_SIZE];
	memset(buffer, 0, BUFFER_SIZE);

	size_t result = _file->ReadChunk(buffer, BUFFER_SIZE);
	if (result == 0 || _file->getOffset() >= static_cast<size_t>(_file->getSize()))
		_processStatus = DONE;
	
	_outputData = std::string(buffer, result);
}

/* ************************************************************************** */
/*                                      handlers                              */
/* ************************************************************************** */

void ProcessRequest::deleteHandler()
{
	if (!_request)
		throw HttpErrorException(500);

	if (_processStatus != HANDLING_METHOD)
		throw HttpErrorException(500);

	std::string path = createPath();

	checkDeleteValidity(path);

	if (unlink(path.c_str()) == -1) {
		throw HttpErrorException(500); }

	std::map<std::string, std::string> headers;
	headers["content-length"] = "0";
	buildResponse(204, headers, "");
	_processStatus = SENDING_HEADERS;
	sendHeaders();
}

void ProcessRequest::getHandler()
{
	if (!_request)
		throw HttpErrorException(500);

	if (_processStatus != HANDLING_METHOD)
		throw HttpErrorException(500);
	
	std::string path = createPath();
	if (!HttpUtils::fileExists(path))
		throw HttpErrorException(404);

	if (HttpUtils::isDirectory(path)) {
		std::string indexFile = createIndexPath(path, _location);
		if (HttpUtils::fileExists(indexFile))
			path = indexFile;
		else if (_location.isAutoIndex()) {
			std::string body = generateAutoIndex(path, _request->getTarget());
			std::map<std::string, std::string> headers;
			headers["content-type"] = "text/html";
			headers["content-length"] = HttpUtils::numberToString(body.length());
			buildResponse(200, headers, body);
			_processStatus = SENDING_HEADERS;
			sendHeaders();
			return ;
		}
		else
			throw HttpErrorException(403);
	}

	if (_file) {
		delete _file;
		_file = NULL;
	}
	_file = new File(path);
	std::map<std::string, std::string> headers;
	headers["content-type"] = _file->getMimeType();
	headers["content-length"] = HttpUtils::numberToString(_file->getSize());
	buildResponse(200, headers, "");

	_processStatus = SENDING_HEADERS;
	sendHeaders();
}

void ProcessRequest::postHandler()
{
	if (!_request)
		throw HttpErrorException(500);

	if (_processStatus != HANDLING_METHOD)
		throw HttpErrorException(500);

	std::string path = createPostPath();

	checkPostValidity(*_request, _location, _server, path);

	std::string filepath = createPostFileName(*_request, path);

	if (_file)
		throw HttpErrorException(500);
	_file = new File(filepath, true);

	_processStatus = WAITING_BODY;
	waitBody();
}

/* ************************************************************************** */
/*                                   selectors                                */
/* ************************************************************************** */

void ProcessRequest::selectLocation()
{
	if (!_request)
		throw HttpErrorException(500);

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
		throw HttpErrorException(404);
	
	_location = locations.find(bestMatch)->second;
}

const std::string & ProcessRequest::selectRoot()
{
	return (!_location.getRoot().empty() ? _location.getRoot() : _server.getRoot());
}

void ProcessRequest::selectHandler()
{
	if (!_request)
		throw HttpErrorException(500);

	const std::string & method = _request->getMethod();
	if (method == "DELETE")
		_handler = &ProcessRequest::deleteHandler;
	else if (method == "POST")
		_handler = &ProcessRequest::postHandler;
	else if (method == "GET")
		_handler = &ProcessRequest::getHandler;
	else
		throw HttpErrorException(405);
	if (!_location.isValidMethod(method))
		throw HttpErrorException(405);
}

std::string ProcessRequest::selectErrorPage(int statusCode)
{
	if (_location.hasErrorPage(statusCode)) {
		return (_location.getErrorPage(statusCode)); }
	if (_server.hasErrorPage(statusCode)) {	
		return (_server.getErrorPage(statusCode)); }
	return ("");
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

/* ************************************************************************** */
/*                              response builder                              */
/* ************************************************************************** */

void ProcessRequest::buildResponse(int statusCode, const std::map<std::string, std::string> & headers, const std::string & body)
{
	_httpResponse = HttpResponse("HTTP/1.1", statusCode, headers, body);
	_httpResponse.addHeader("date", HttpUtils::getCurrentDate());
	_httpResponse.addHeader("connection", "close");
	std::ostringstream oss;
	std::vector<std::string>::const_iterator cit = _server.getServerNames().begin();
	if (cit != _server.getServerNames().end()) {
		oss << *cit;
		++cit;
		for (; cit != _server.getServerNames().end(); ++cit) {
			oss << " " << *cit; }}
	_httpResponse.addHeader("server", oss.str());
}

void ProcessRequest::buildRedirect()
{
	std::string redirectPath = _location.getRedirectPath();
	int redirectCode = _location.getRedirectCode();

	std::string body =
		"<html><body><h1>" + HttpUtils::httpStatusMessage(redirectCode) + "</h1>\n"
		"<p>Redirecting to <a href=\"" + redirectPath + "\">" + redirectPath + "</a></p>\n"
		"</body></html>";

	std::map<std::string, std::string> headers;
	headers["content-type"] = "text/html";
	headers["location"] = redirectPath;
	headers["content-length"] = HttpUtils::numberToString(body.length());

	if (_file) {
		delete _file;
		_file = NULL;
	}
	buildResponse(redirectCode, headers, body);
	_processStatus = SENDING_HEADERS;
}

// void ProcessRequest::buildError(int statusCode, const ServerConfig & server, const Location * location)
// {
// 	std::string filePath = selectErrorPage(statusCode, server, location);
// 	std::string body;
// 	std::string mimeType;
// 	try {
// 		body = HttpUtils::readFile(filePath);
// 		mimeType = HttpUtils::getMimeType(filePath); }
// 	catch (const std::exception &e) {
// 		body = "<html><body><h1>" + HttpUtils::numberToString(statusCode) + " " +
// 			HttpUtils::httpStatusMessage(statusCode) + "</h1></body></html>";
// 		mimeType = "text/html"; }
// 	std::map<std::string, std::string> headers;
// 	headers["Content-Type"] = mimeType;

// 	if (statusCode == 405) {
// 		headers["allow"] = createAllowedMethodsList(*location); }

// 	_httpResponse = HttpResponse("HTTP/1.1", statusCode, headers, body);
// 	addMandatoryHeaders();
// }

void ProcessRequest::selectServer()
{
	if (!_request)
		throw HttpErrorException(500);
	
	const std::map<std::string, std::string> & headers = _request->getHeaders();
	std::map<std::string, std::string>::const_iterator headersCit = headers.find("host"); 
	if (headersCit != headers.end()) {
		std::vector<ServerConfig>::const_iterator serverCit = _serversVector.begin();
		for (; serverCit != _serversVector.end(); ++serverCit) {
			if (serverCit->hasServerName(headersCit->second)) {
				_server =  *serverCit;
				return ;
			}
		}
	}
	if (_serversVector.empty()) {
		throw HttpErrorException(500);
	}
	_server = _serversVector[0];
}

/* ************************************************************************** */
/*                            private utils methods                           */
/* ************************************************************************** */

void ProcessRequest::checkMethodValidity()
{
	const std::vector<std::string> & allowedMethods = _location.getMethods();
	std::vector<std::string>::const_iterator cit = allowedMethods.begin();
	for (; cit != allowedMethods.end(); ++cit) {
		if (*cit == _request->getMethod())
			return ;
	}
	throw HttpErrorException (405);
}

/* ************************************************************************** */
/*                            non member functions                            */
/* ************************************************************************** */

static void checkDeleteValidity(const std::string & path)
{
	if (!HttpUtils::fileExists(path)) {
		throw HttpErrorException(404); }
	if (!HttpUtils::isRegularFile(path)) {
		throw HttpErrorException(403); }
	if (!HttpUtils::hasWritePermission(path)) {
		throw HttpErrorException(403); }
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
	if (!dir) {
		throw HttpErrorException(500); }
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
	html << "</ul></body></html>";
	return (html.str());
}

static void checkPostValidity(
	const HttpRequest & request, const Location & location ,
	const ServerConfig & server, const std::string & path)
{
	if (location.getUploadPath().empty()) {
		throw HttpErrorException(403); }
	if (request.getContentLength() > (server.getClientMaxBodySize())) {
		throw HttpErrorException(413); }
	if (!HttpUtils::fileExists(path)) {
		throw HttpErrorException(404); }
	if (!HttpUtils::isDirectory(path)) {
		throw HttpErrorException(403); }
	if (!HttpUtils::hasWritePermission(path)) {
		throw HttpErrorException(403); }
}

static std::string sanitizeFilenamePart(const std::string & input)
{
	std::string result;
	for (size_t i = 0; i < input.length(); ++i) {
		char c = input[i];
		if (std::isalnum(c) || c == '-' || c == '_') {
			result += c; }
		else {
			result += '_'; }
		if (result.size() > 39) {
			break ; }}
	return (result);
}

static std::string createPostFileName(
	const HttpRequest & request, const std::string & path)
{
	std::string extension;
	if (request.hasHeader("content-type")) {
		extension = HttpUtils::getExtensionFromMimeType(request.getHeaderValue("content-type")); }

	std::string userAgent;
	if (request.hasHeader("user-agent")) {
		userAgent = "ua_" + sanitizeFilenamePart(request.getHeaderValue("user-agent")) + "_"; }
	else {
		userAgent = "ua_unknown_"; }

	std::string filename = path + "/upload_" + userAgent + HttpUtils::generateUniqueTimestamp();
	if (!extension.empty()) {
		filename += "." + extension; }
	return (filename);
}

// static std::string createPath(const std::string & root, const std::string & subpath)
// {
// 	std::string cleanRoot = root;
// 	HttpUtils::trimFinalSlash(cleanRoot);
// 	std::string cleanSubpath = subpath;
// 	HttpUtils::trimSlashes(cleanSubpath);
	
// 	return (cleanRoot + '/' + cleanSubpath);
// }

std::string ProcessRequest::createPath()
{
	if (!_request)
		throw HttpErrorException(500);

	std::string locationPath = _location.getPath();
	std::string target = _request->getTarget();
	std::string root = selectRoot();

	if (target.find(locationPath) != 0)
		throw HttpErrorException(404);

	std::string relativePath = target.substr(locationPath.size());
	HttpUtils::trimFinalSlash(root);
	HttpUtils::trimSlashes(relativePath);

	return (root + '/' + relativePath);
}

std::string ProcessRequest::createPostPath()
{
	if (!_request)
		throw HttpErrorException(500);

	std::string root = selectRoot();
	std::string locationPath = _location.getUploadPath();
	HttpUtils::trimFinalSlash(root);
	HttpUtils::trimSlashes(locationPath);

	return (root + '/' + locationPath);
}

