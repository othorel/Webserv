#include <string>
#include <sstream>
#include <fstream>
#include <stdexcept>
#include "../../include/http/ResponseBuilder.hpp"
#include "../../include/http/HttpRequest.hpp"
#include "../../include/http/HttpResponse.hpp"
#include "../../include/config/Location.hpp"
# include "../../include/handlers/AHandler.hpp"
# include "../../include/handlers/GetHandler.hpp"
#include "../../include/handlers/PostHandler.hpp"
#include "../../include/handlers/DeleteHandler.hpp"
#include "../../include/handlers/CgiHandler.hpp"


/* ************************************************************************** */
/*                            non member functions                            */
/* ************************************************************************** */

/* ****************************** utils ************************************* */

std::string httpStatusMessage(int code);

static std::string numberToString(size_t value)
{
	std::ostringstream oss;
	oss << value;
	return (oss.str());
}

static std::string numberToString(int value)
{
	std::ostringstream oss;
	oss << value;
	return (oss.str());
}

static std::string readFile(const std::string & path)
{
	std::ifstream file(path.c_str());
	if (!file) {
		throw std::runtime_error("Could not open file " + path); }
	std::ostringstream content;
	content << file.rdbuf();
	file.close();
	return (content.str());
}

static const Location & findMatchinglocation(
		const std::map<std::string, Location> & locations,
		const std::string & target)
{
	std::string bestMatch = "";
	std::map<std::string, Location>::const_iterator it;
	for (it = locations.begin(); it != locations.end(); ++it)
	{
		const std::string& path = it->first;
		if (target.compare(0, path.size(), path) == 0)
		{
			if (path.size() > bestMatch.size())
				bestMatch = path;
		}
	}
	if (bestMatch.empty())
		throw ResponseBuilder::HttpErrorException(404);
	return (locations.find(bestMatch)->second);
}

static std::string selectErrorPage(int statusCode, const ServerConfig & server, const Location * location)
{
	if (location && location->hasErrorPage(statusCode)) {	// hasErrorCode à implementer
		return (location->getErrorPage(statusCode)); }		// getErrorPage à implementer
	if (server.hasErrorPage(statusCode)) {					// hasErrorCode à implementer
		return (server.getErrorPage(statusCode)); }			// getErrorPage à implementer
	return ("");
}

/* *************************** handler factory ****************************** */

typedef IHandler * (*HandlerFactoryFn)();

static IHandler * selectHandler(const HttpRequest& request, const Location & location)
{
	if (location.hasCgi())
		return (new CgiHandler());

	static std::map<std::string, HandlerFactoryFn> handlerFactories;
	if (handlerFactories.empty()) {
		handlerFactories["GET"] = &createGetHandler;
		handlerFactories["POST"] = &createPostHandler;
		handlerFactories["DELETE"] = &createDeleteHandler; }

	if (handlerFactories.find(request.getMethod()) != handlerFactories.end())
		return (handlerFactories[request.getMethod()]());
	throw ResponseBuilder::HttpErrorException(405);
}

static IHandler * createGetHandler()
{
	return (new GetHandler());
}

static IHandler * createPostHandler()
{
	return (new PostHandler());
}

static IHandler * createDeleteHandler()
{
	return (new DeleteHandler());
}

static IHandler * createCgiHandler()
{
	return (new CgiHandler());
}

/* ************************************************************************** */
/*                                  constructors                              */
/* ************************************************************************** */

ResponseBuilder::ResponseBuilder() :
	_httpResponse()
{}

ResponseBuilder::ResponseBuilder(const HttpRequest& request, const ServerConfig & server)
{
	_httpResponse = buildResponse(request, server);
}

ResponseBuilder::ResponseBuilder(const ResponseBuilder & other) :
	_httpResponse(other._httpResponse)
{}

/* ************************************************************************** */
/*                                    operators                               */
/* ************************************************************************** */

ResponseBuilder & ResponseBuilder::operator=(const ResponseBuilder & other)
{
	if (&other != this) {
		this->_httpResponse = other._httpResponse;
	}
	return (*this);
}

/* ************************************************************************** */
/*                                   destructor                               */
/* ************************************************************************** */

ResponseBuilder::~ResponseBuilder()
{}

/* ************************************************************************** */
/*                              response builder                              */
/* ************************************************************************** */

const HttpResponse & ResponseBuilder::buildResponse(const HttpRequest& request, const ServerConfig & server)
{
	IHandler * handler = NULL;
	const Location * locationPtr = NULL;
	const std::map<std::string, Location> & locations = server.getLocations();
	try {
		locationPtr = &findMatchinglocation(locations, request.getTarget());
		if (locationPtr->hasRedirect()) {
			buildRedirect(locationPtr->getRedirectCode(), locationPtr->getRedirectPath());
			return (_httpResponse); }
		std::string method = request.getMethod();
		if (!locationPtr->isValidMethod(method)) {
			throw HttpErrorException(405); }
		try {
			handler = selectHandler(request, *locationPtr);
			_httpResponse = handler->handle(request, *locationPtr, server);
			delete handler; }
		catch (...) {
			delete handler;
			throw; }}
	catch (const HttpErrorException & e) {
		buildError(e.getStatusCode(), server, locationPtr); }
	return (_httpResponse);
}

void ResponseBuilder::buildRedirect(int code, const std::string & path)
{
	std::string body = "<html><body><h1>" + httpStatusMessage(code) + "</h1>"
        "<p>Redirecting to <a href=\"" + path + "\">" + path + "</a></p></body></html>";
	std::map<std::string, std::string> headers;
	headers["Location"] = path;
	headers["Content-Type"] = "text/html";
	headers["Content-Length"] = numberToString(body.size());
	_httpResponse = HttpResponse("HTTP/1.1", code, headers, body);
}

void ResponseBuilder::buildError(int statusCode, const ServerConfig & server, const Location * location)
{
	std::string filePath = selectErrorPage(statusCode, server, location);
	std::string body;
	try {
		body = readFile(filePath); }
	catch (const std::exception &e) {
		// std::cerr << "Error page not found: " << e.what() << std::endl;
		body = "<html><body><h1>" + numberToString(statusCode) + " " +
			httpStatusMessage(statusCode) + "</h1></body></html>"; }
	std::map<std::string, std::string> headers;
	headers["Content-Type"] = "text/html";
	headers["Content-Length"] = numberToString(body.size());
	_httpResponse = HttpResponse("HTTP/1.1", statusCode, headers, body);
}
