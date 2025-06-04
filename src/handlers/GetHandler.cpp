#include <string>
#include "../../include/handlers/GetHandler.hpp"
#include "../../include/http/HttpResponse.hpp"

/* ************************************************************************** */
/*                                   constructors                             */
/* ************************************************************************** */

GetHandler::GetHandler()
{}

GetHandler::GetHandler(const GetHandler & other)
{}

/* ************************************************************************** */
/*                                    operators                               */
/* ************************************************************************** */

GetHandler & GetHandler::operator=(const GetHandler & other)
{
	return (*this);
}

/* ************************************************************************** */
/*                                   destructors                              */
/* ************************************************************************** */

GetHandler::~GetHandler()
{}

/* ************************************************************************** */
/*                                      handler                               */
/* ************************************************************************** */

HttpResponse GetHandler::handle(const HttpRequest & request, const Location & location , const ServerConfig & server)
{
	std::string path = selectRoot(server, location) + request.getTarget();
	if (!fileExists(path)) {
		throw HttpErrorException(404); }
	if (isDirectory(path)) {
		std::string indexFile = createIndexPath(path, location);
		if (fileExists(indexFile)) {
			path = indexFile; }
		else if (location.isAutoIndex()) {
			std::string body = generateAutoIndex(path, request.getTarget());
			std::map<std::string, std::string> headers;
			headers["Content-Type"] = "text/html";
			headers["Content-Length"] = numberToString(body.size());
			HttpResponse httpResponse("HTTP/1.1", 200, headers, body);
			return (httpResponse); }
		else
			throw HttpErrorException(403); }
	std::string body = readFile(path);
	std::map<std::string, std::string> headers;
	headers["Content-Type"] = "text/html";
	headers["Content-Length"] = numberToString(body.size());
	HttpResponse httpResponse("HTTP/1.1", 200, headers, body);
	return (httpResponse);
}
