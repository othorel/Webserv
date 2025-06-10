#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <stdexcept>
#include <unistd.h>
#include <dirent.h>
#include "../../include/http/ProcessRequest.hpp"
# include "../../include/http/HttpErrorException.hpp"
# include "../../include/http/HttpUtils.hpp"
#include "../../include/http/HttpRequest.hpp"
#include "../../include/http/HttpResponse.hpp"
#include "../../include/config/Location.hpp"
#include "../../include/config/ServerConfig.hpp"

/* ************************************************************************** */
/*                                  constructors                              */
/* ************************************************************************** */

ProcessRequest::ProcessRequest() :
	_processStatus(DONE),
	_request(),
	_server(),
	_location(),
	_handler(NULL),
	_file(NULL),
	_httpResponse(),
	_rawString()
{}

ProcessRequest::ProcessRequest(const HttpRequest& request, std::vector<ServerConfig> serverVector) :
	_processStatus(READY),
	_request(request),
	_file(NULL),
	_httpResponse(),
	_rawString()
{
		selectServer(serverVector);
		selectLocation();
		std::string method = request.getMethod();
		if (!_location.isValidMethod(method))
			throw HttpErrorException(405);
		selectHandler();
		_httpResponse = HttpResponse();
		if (_location.hasRedirect()) 
			buildRedirect();
}

ProcessRequest::ProcessRequest(const ProcessRequest & other) :
	_processStatus(other._processStatus),
	_request(other._request),
	_server(other._server),
	_location(other._location),
	_handler(other._handler),
	_httpResponse(other._httpResponse),
	_rawString(other._rawString)
{
	_file = other._file ? new File(*other._file) : NULL;
}

void ProcessRequest::reset(const HttpRequest & request, const std::vector<ServerConfig> & serverVector)
{
	if (_file)
	{
		delete _file;
		_file = NULL;
	}

	_processStatus = READY;
	_request = request;
	_rawString.clear();
	_httpResponse = HttpResponse();
	selectServer(serverVector);
	selectLocation();
	std::string method = _request.getMethod();
	if (!_location.isValidMethod(method))
		throw HttpErrorException(405);
	selectHandler();
	if (_location.hasRedirect())
		buildRedirect();
}

/* ************************************************************************** */
/*                                    operators                               */
/* ************************************************************************** */

ProcessRequest & ProcessRequest::operator=(const ProcessRequest & other)
{
	if (&other != this) {
		_processStatus = other._processStatus;
		_request = other._request;
		_server = other._server;
		_location = other._location;
		_handler = other._handler;
		_httpResponse = other._httpResponse;
		_rawString = other._rawString;
		if (_file)
			delete _file;
		_file = other._file ? new File(*other._file) : NULL;
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
}

/* ************************************************************************** */
/*                                   selectors                                */
/* ************************************************************************** */

void ProcessRequest::selectServer(const std::vector<ServerConfig> & serverVector)
{
	const std::map<std::string, std::string> & headers = _request.getHeaders();
	std::map<std::string, std::string>::const_iterator headersCit = headers.find("host"); 
	if (headersCit != headers.end()) {
		std::vector<ServerConfig>::const_iterator serverCit = serverVector.begin();
		for (; serverCit != serverVector.end(); ++serverCit) {
			if (serverCit->hasServerName(headersCit->second)) {
				_server =  *serverCit;
				return ;
			}
		}
	}
	if (serverVector.empty()) {
		throw HttpErrorException(500);
	}
	_server = serverVector[0];
}

void ProcessRequest::selectLocation()
{
	const std::map<std::string, Location> & locations = _server.getLocations();
	const std::string & target = _request.getTarget();
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
	const std::string & method = _request.getMethod();
	if (method == "DELETE")
		_handler = &ProcessRequest::deleteHandler;
	else if (method == "POST")
		_handler = &ProcessRequest::postHandler;
	else if (method == "GET")
		_handler = &ProcessRequest::getHandler;
	else
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

/* ************************************************************************** */
/*                                     process                                */
/* ************************************************************************** */

// When _processStatus is READY
void ProcessRequest::process()
{
	if (_handler)
		(this->*_handler)();
}

// When _processStatus is WAITING_BODY
size_t ProcessRequest::receiveBodyChunk(char * buffer, size_t writesize)
{
	if (_processStatus != WAITING_BODY || !_file || !buffer)
		throw HttpErrorException(500);

	size_t remainingBytes = _request.getContentLength() - _file->getOffset();
	size_t bytesToWrite = (writesize > remainingBytes) ? remainingBytes : writesize;

	size_t result = _file->WriteChunk(buffer, bytesToWrite);

	if (_file->getOffset() >= _request.getContentLength()) {
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
		_processStatus = RESPONSE_READY;
	}
	return (result);
}

// When _processStatus is RESPONSE_READY
const std::string & ProcessRequest::sendHttpResponse()
{
	if (_processStatus != RESPONSE_READY)
		throw HttpErrorException(500);
	if (_file != NULL)
		_processStatus = SENDING_BODY;
	else
		_processStatus = DONE;
	_rawString = _httpResponse.toRawString();
	return (_rawString);
}

// When _processStatus is SENDING_BODY
size_t ProcessRequest::sendBodyChunk(char * buffer, size_t readsize)
{
	if (_processStatus != SENDING_BODY || !_file  || !buffer)
		throw HttpErrorException(500);
	size_t result = _file->ReadChunk(buffer, readsize);
	if (result == 0 || _file->getOffset() >= _file->getSize())
		_processStatus = DONE;
	return (result);
}

/* ************************************************************************** */
/*                                      handlers                              */
/* ************************************************************************** */

void ProcessRequest::deleteHandler()
{
	if (_processStatus != READY)
		throw HttpErrorException(500);

	std::string path = selectRoot() + _request.getTarget();

	checkDeleteValidity(path);

	if (unlink(path.c_str()) == -1) {
		throw HttpErrorException(500); }

	std::map<std::string, std::string> headers;
	headers["content-length"] = "0";
	buildResponse(204, headers, "");
	if (_file) {
		delete _file;
		_file = NULL;
	}
	_processStatus = RESPONSE_READY;
}

void ProcessRequest::getHandler()
{
	if (_processStatus != READY)
		throw HttpErrorException(500);
	
	std::string path = selectRoot();
	if (!HttpUtils::fileExists(path))
		throw HttpErrorException(404);

	if (HttpUtils::isDirectory(path)) {
		std::string indexFile = createIndexPath(path, _location);
		if (HttpUtils::fileExists(indexFile))
			path = indexFile;
		else if (_location.isAutoIndex()) {
			std::string body = generateAutoIndex(path, _request.getTarget());
			std::map<std::string, std::string> headers;
			headers["content-type"] = "text/html";
			headers["content-length"] = HttpUtils::numberToString(body.length());
			buildResponse(200, headers, body);
			if (_file) {
				delete _file;
				_file = NULL;
			}
			_processStatus = RESPONSE_READY;
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

	_processStatus = RESPONSE_READY;
}

void ProcessRequest::postHandler()
{
	if (_processStatus != READY)
		throw HttpErrorException(500);

	std::string root = selectRoot();
	std::string uploadPath = _location.getUploadPath();
	if (uploadPath.empty())
		throw HttpErrorException(403);
	HttpUtils::trimFinalSlash(root);
	HttpUtils::trimSlashes(uploadPath);
	
	std::string path = root + '/' + uploadPath;

	checkPostValidity(_request, _location, _server, path);

	std::string filepath = createPostFileName(_request, _server, path);

	if (_file)
		throw HttpErrorException(500);
	_file = new File(filepath, true);

	_processStatus = WAITING_BODY;
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
	_processStatus = RESPONSE_READY;
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


/* ************************************************************************** */
/*                            non member functions                            */
/* ************************************************************************** */

static std::string createAllowedMethodsList(const Location & location)
{
	std::ostringstream oss;
	const std::vector<std::string> & methodsVector = location.getMethods();
	std::vector<std::string>::const_iterator cit = methodsVector.begin();
	if (cit != methodsVector.end()) {
		oss << *cit;
		cit++; }
	for (; cit != methodsVector.end(); cit++) {
		oss << " ";
		oss << *cit; }
	return (oss.str());
}

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
	const HttpRequest & request, const ServerConfig & server, const std::string & path)
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
