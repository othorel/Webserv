

#include <map>
#include "../../include/http/RequestHandlers.hpp"
#include "../../include/http/ProcessRequest.hpp"
#include "../../include/http/HttpErrorException.hpp"

std::map<std::string, RequestHandlers::HandlerFunction> RequestHandlers::_methodList;

/* ************************************************************************** */
/*                                   selection                                */
/* ************************************************************************** */

RequestHandlers::HandlerFunction RequestHandlers::selectHandler(const std::string & method)
{
	if (_methodList.empty()) {
		_methodList["GET"] = &RequestHandlers::getHandler;
		_methodList["POST"] = &RequestHandlers::postHandler;
		_methodList["DELETE"] = &RequestHandlers::deleteHandler;
	}

	std::map<std::string, RequestHandlers::HandlerFunction>::const_iterator cit = _methodList.find(method);
	if (cit != _methodList.end())
		return (cit->second);
	return (NULL);
}

/* ************************************************************************** */
/*                                      handlers                              */
/* ************************************************************************** */

void RequestHandlers::deleteHandler(ProcessRequest * req)
{
	if (!req->_request)
		throw HttpErrorException(500);

	if (req->_processStatus != HANDLING_METHOD)
		return ;

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

void RequestHandlers::getHandler(ProcessRequest * req)
{
	if (!req->_request)
		throw HttpErrorException(500);

	if (req->_processStatus != HANDLING_METHOD)
		return ;
	
	std::string path = createPath();
	if (!HttpUtils::fileExists(path))
		throw HttpErrorException(404);

	if (HttpUtils::isDirectory(path)) {
		std::string indexFile = createIndexPath(path, req->_location);
		if (HttpUtils::fileExists(indexFile))
			path = indexFile;
		else if (req->_location.isAutoIndex()) {
			std::string body = generateAutoIndex(path, req->_request->getTarget());
			std::map<std::string, std::string> headers;
			headers["content-type"] = "text/html";
			headers["content-length"] = HttpUtils::numberToString(body.length());
			buildResponse(200, headers, body);
			req->_processStatus = SENDING_HEADERS;
			sendHeaders();
			return ;
		}
		else
			throw HttpErrorException(403);
	}

	if (req->_file) {
		delete req->_file;
		req->_file = NULL;
	}
	req->_file = new File(path);
	std::map<std::string, std::string> headers;
	headers["content-type"] = req->_file->getMimeType();
	headers["content-length"] = HttpUtils::numberToString(req->_file->getSize());
	buildResponse(200, headers, "");

	_processStatus = SENDING_HEADERS;
	sendHeaders();
}

void RequestHandlers::postHandler(ProcessRequest * req)
{
	if (!req->_request)
		throw HttpErrorException(500);

	if (req->_processStatus != HANDLING_METHOD)
		return ;

	if (req->_file)
		throw HttpErrorException(500);
	std::string path;
	if (req->_request->hasHeader("content-type")
		&& req->_request->getHeaderValue("content-type").find("multipart/form-data") == 0
		&& req->_request->getHeaderValue("content-type").find("boundary") != std::string::npos) {
		path = createUploadPath();
		checkPostValidity(path);
		std::string filepath = createUploadFilename(*req->_request, path);
		req->_file = new File(filepath, true);
	}
	else {
		path = createPath();
		checkPostValidity(path);
	}

	req->_processStatus = WAITING_BODY;
	waitBody();
}

/* ************************************************************************** */
/*                           utils private methods                            */
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