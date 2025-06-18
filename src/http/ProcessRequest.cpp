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
#include "../../include/cgi/CGIHandler.hpp"

static void checkDeleteValidity(const std::string & path);
static std::string createIndexPath(std::string path, const Location & location);
static std::string generateAutoIndex(const std::string & dirPath, const std::string & uriPath);
static std::string sanitizeFilenamePart(const std::string & input);
static std::string createUploadFilename(
	const HttpRequest & request, const std::string & path);
static bool isCgi(const std::string & path);

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
	_handler(NULL),
	_request(NULL),
	_file(NULL)
{
	if (serversVector.empty())
		throw HttpErrorException(500);
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
	_handler(other._handler)
{
	_file = other._file ? new File(*other._file) : NULL;
	_request = other._request ? new HttpRequest(*other._request) : NULL;
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
	_serverTimeout = 0;
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
		if (_file)
			delete _file;
		_file = other._file ? new File(*other._file) : NULL;
		if (_request)
			delete _request;
		_request = other._request ? new HttpRequest(*other._request) : NULL;
	}
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

	if (_inputData.size() > MAX_HEADERS_SIZE)
		throw HttpErrorException(431);
	size_t pos = _inputData.find("\r\n\r\n");
	if (pos != std::string::npos) {
		std::string headersPart = _inputData.substr(0, pos + 4);
		std::string bodyPart = _inputData.substr(pos + 4);
		RequestParser parser(headersPart);
		_request = parser.release();

		if (_request->getContentLength() > _server.getClientMaxBodySize())
			throw HttpErrorException(413);
		
		_inputData = bodyPart;
		selectServer();
		_serverTimeout = _server.getSessionTimeout();
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

	_handler = RequestHandlers::selectHandler(_request->getMethod());
	if (!_handler)
		throw HttpErrorException(501);
	(this->_handler)();
}

// From status WAITING_BODY to SENDING_HEADERS
void ProcessRequest::waitBody()
{
	// debug
	std::cout << "WAITBODY" << std::endl;

	if (_processStatus  != WAITING_BODY)
		return ;

	if (!_request)
		throw HttpErrorException(500);

	// if body is a file to upload
	if (_file) {
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
	// if body is not a file to upload
	else {
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
					addFinalHeaders();
					_processStatus = SENDING_HEADERS;
					sendHeaders();
					return ;
			}
			_processStatus = SENDING_HEADERS;
			sendHeaders();
		}
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
		throw HttpErrorException(500);
	
	char buffer[BUFFER_SIZE];
	memset(buffer, 0, BUFFER_SIZE);

	size_t result = _file->ReadChunk(buffer, BUFFER_SIZE);
	if (result == 0 || _file->getOffset() >= static_cast<size_t>(_file->getSize()))
		_processStatus = DONE;
	
	_outputData = std::string(buffer, result);
}

/* ************************************************************************** */
/*                                      cgi                                   */
/* ************************************************************************** */

void ProcessRequest::cgiGetHandler()
{
	std::string path = createPath();

	CGIHandler cgi(*_request, path);
	_httpResponse = cgi.getHttpResponse();
	addFinalHeaders();

	_processStatus = SENDING_HEADERS;
	sendHeaders();
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
	if (isCgi(_request->getTarget()) && method == "GET")
		_handler = &ProcessRequest::cgiGetHandler;
	else if (method == "DELETE")
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

int ProcessRequest::getServerTimeout() const
{
	return (_serverTimeout);
}

bool ProcessRequest::closeConection()
{
	if (!_request || !_request->hasHeader("connection"))
		return (false);
	if (_request->getHeaderValue("connection") == "close")
		return (true);
	return (false);
}

/* ************************************************************************** */
/*                              response builder                              */
/* ************************************************************************** */

void ProcessRequest::buildResponse(int statusCode, const std::map<std::string, std::string> & headers, const std::string & body)
{
	_httpResponse = HttpResponse("HTTP/1.1", statusCode, headers, body);
	addFinalHeaders();
}

void ProcessRequest::addFinalHeaders()
{
	if (_httpResponse.getHeaders().find("date") == _httpResponse.getHeaders().end())
		_httpResponse.addHeader("date", HttpUtils::getCurrentDate());
	if (_httpResponse.getHeaders().find("connection") == _httpResponse.getHeaders().end()) {
		if (_request && _request->hasHeader("connection") && _request->getHeaderValue("connection") == "close")
			_httpResponse.addHeader("connection", "close");
		else
			_httpResponse.addHeader("connection", "keep-alive");
	}
	if (_httpResponse.getHeaders().find("server") == _httpResponse.getHeaders().end()) {
		std::ostringstream oss;
		std::vector<std::string>::const_iterator cit = _server.getServerNames().begin();
		if (cit != _server.getServerNames().end()) {
			oss << *cit;
			++cit;
			for (; cit != _server.getServerNames().end(); ++cit) {
				oss << " " << *cit; }}
		_httpResponse.addHeader("server", oss.str());
	}
	if (_httpResponse.getHeaders().find("content-length") == _httpResponse.getHeaders().end())
		_httpResponse.addHeader("content-length", HttpUtils::numberToString(_httpResponse.getBody().size()));
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

	// debug
	std::cout << "SERVER VECTOR : " << std::endl;
	std::vector<ServerConfig>::const_iterator cit = _serversVector.begin();
	for (; cit != _serversVector.end(); ++cit) {
		std::cout << "server listening : " << cit->getListen().first << ":" << cit->getListen().second << std::endl;
	}
	
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

std::string ProcessRequest::createUploadPath()
{
	if (!_request)
		throw HttpErrorException(500);

	std::string root = selectRoot();
	std::string locationPath = _location.getUploadPath();
	HttpUtils::trimFinalSlash(root);
	HttpUtils::trimSlashes(locationPath);

	return (root + '/' + locationPath);
}

void ProcessRequest::checkPostValidity(const std::string & path)
{
	if (_file) {
		if (_location.getUploadPath().empty())
			throw HttpErrorException(500);
		if (!HttpUtils::isDirectory(path))
			throw HttpErrorException(500);
		if (!HttpUtils::hasWritePermission(path))
			throw HttpErrorException(403);
	}
	else {
		if (!HttpUtils::fileExists(path))
			throw HttpErrorException(404);
	}
	if (_request->getContentLength() > (_server.getClientMaxBodySize())) {
		throw HttpErrorException(413); }
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

static std::string createUploadFilename(
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

static bool isCgi(const std::string & path)
{
	size_t start = path.find_last_of('.');
	if (start == std::string::npos)
		return (false);
	size_t end = path.find('?', start);
	std::string ext = path.substr(start, end - start);
	return (ext == ".py" || ext == ".php" || ext == ".pl");
}
