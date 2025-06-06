#include <string>
#include <sstream>
#include <fstream>
#include <stdexcept>
# include "../../include/http/HttpErrorException.hpp"
# include "../../include/http/HttpUtils.hpp"
#include "../../include/http/ResponseBuilder.hpp"
#include "../../include/http/HttpRequest.hpp"
#include "../../include/http/HttpResponse.hpp"
#include "../../include/config/Location.hpp"
#include "../../include/config/ServerConfig.hpp"
# include "../../include/http/handlers/AHandler.hpp"
# include "../../include/http/handlers/GetHandler.hpp"
#include "../../include/http/handlers/PostHandler.hpp"
// #include "../../include/http/handlers/DeleteHandler.hpp"
// #include "../../include/http/handlers/CgiHandler.hpp"

static AHandler * selectHandler(const HttpRequest& request, const Location & location);
static AHandler * createGetHandler();

/* ************************************************************************** */
/*                                  constructors                              */
/* ************************************************************************** */

ResponseBuilder::ResponseBuilder() :
	_httpResponse()
{}

ResponseBuilder::ResponseBuilder(const HttpRequest& request, std::vector<ServerConfig> serverVector)
{
	_httpResponse = buildResponse(request, serverVector);
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

const HttpResponse & ResponseBuilder::buildResponse(const HttpRequest& request, const std::vector<ServerConfig> serverVector)
{
	AHandler * handler = NULL;
	const Location * locationPtr = NULL;
	const ServerConfig server = selectServer(request, serverVector);
	const std::map<std::string, Location> & locations = server.getLocations();
	try {
		locationPtr = &findMatchinglocation(locations, request.getTarget());
		if (locationPtr->hasRedirect()) {
			buildRedirect(locationPtr->getRedirectCode(), server, locationPtr);
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
	addMandatoryHeaders(server, _httpResponse.getBody().size());
	return (_httpResponse);
}

void ResponseBuilder::buildRedirect(int statusCode, const ServerConfig & server, const Location * location)
{
	std::string path = (*location).getRedirectPath();
	std::string filePath = selectErrorPage(statusCode, server, location);
	std::string body;
	std::string mimeType;
	try {
		body = HttpUtils::readFile(filePath);
		mimeType = HttpUtils::getMimeType(filePath); }
	catch (const std::exception &e) {
		body = "<html><body><h1>" + HttpUtils::httpStatusMessage(statusCode) + "</h1>"
        "<p>Redirecting to <a href=\"" + path + "\">" + path + "</a></p></body></html>";
		mimeType = "text/html"; }
	std::map<std::string, std::string> headers;
	headers["Content-Type"] = mimeType;
	headers["Location"] = path;
	_httpResponse = HttpResponse("HTTP/1.1", statusCode, headers, body);
	addMandatoryHeaders(server, _httpResponse.getBody().size());
}

// ancienne fonction  supprimer si la nouvelle fonctionne correctement
// void ResponseBuilder::buildRedirect(int statusCode, const std::string & path, const ServerConfig & server)
// {
// 	std::string body = "<html><body><h1>" + HttpUtils::httpStatusMessage(statusCode) + "</h1>"
//         "<p>Redirecting to <a href=\"" + path + "\">" + path + "</a></p></body></html>";
// 	std::map<std::string, std::string> headers;
// 	headers["Location"] = path;
// 	headers["Content-Type"] = "text/html";
// 	addMandatoryHeaders(server, _httpResponse.getBody().size());
// 	_httpResponse = HttpResponse("HTTP/1.1", statusCode, headers, body);
// }

void ResponseBuilder::buildError(int statusCode, const ServerConfig & server, const Location * location)
{
	std::string filePath = selectErrorPage(statusCode, server, location);
	std::string body;
	std::string mimeType;
	try {
		body = HttpUtils::readFile(filePath);
		mimeType = HttpUtils::getMimeType(filePath); }
	catch (const std::exception &e) {
		body = "<html><body><h1>" + HttpUtils::numberToString(statusCode) + " " +
			HttpUtils::httpStatusMessage(statusCode) + "</h1></body></html>";
		mimeType = "text/html"; }
	std::map<std::string, std::string> headers;
	headers["Content-Type"] = mimeType;

	if (statusCode == 405) {
		headers["Allow"] = createAllowedMethodsList(*location); }

	_httpResponse = HttpResponse("HTTP/1.1", statusCode, headers, body);
	addMandatoryHeaders(server, _httpResponse.getBody().size());
}

/* ************************************************************************** */
/*                                   getters                                  */
/* ************************************************************************** */

const HttpResponse & ResponseBuilder::getHttpResponse() const
{
	return (_httpResponse);
}

/* ************************************************************************** */
/*                                   setters                                  */
/* ************************************************************************** */

void ResponseBuilder::addMandatoryHeaders(const ServerConfig & server, size_t bodySize)
{
	_httpResponse.addHeader("Content-Length", HttpUtils::numberToString(bodySize));
	_httpResponse.addHeader("Date", HttpUtils::getCurrentDate());
	std::ostringstream oss;
	std::vector<std::string>::const_iterator cit = server.getServerNames().begin();
	if (cit != server.getServerNames().end()) {
	oss << *cit;
	++cit;
	for (; cit != server.getServerNames().end(); ++cit) {
		oss << " " << *cit; }}
	_httpResponse.addHeader("Server", oss.str());
}

/* ************************************************************************** */
/*                            non member functions                            */
/* ************************************************************************** */

/* ****************************** utils ************************************* */

static const ServerConfig & selectServer(const HttpRequest& request, const std::vector<ServerConfig> & serverVector)
{
	std::map<std::string, std::string> headers = request.getHeaders();
	std::map<std::string, std::string>::const_iterator headersCit = headers.find("host"); 
	if (headersCit != headers.end()) {
		std::vector<ServerConfig>::const_iterator serverCit = serverVector.begin();
		for (; serverCit != serverVector.end(); serverCit++) {
			if (serverCit->hasServerName(headersCit->second)) {
				return (*serverCit); }}}
	if (serverVector.empty()) {
		throw std::runtime_error("No server available to select");
	}
	return (serverVector[0]);
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
		throw HttpErrorException(404);
	
	return (locations.find(bestMatch)->second);
}

static std::string selectErrorPage(int statusCode, const ServerConfig & server, const Location * location)
{
	if (location && location->hasErrorPage(statusCode)) {
		return (location->getErrorPage(statusCode)); }
	if (server.hasErrorPage(statusCode)) {	
		return (server.getErrorPage(statusCode)); }
	return ("");
}

// version for testing only
// static std::string selectErrorPage(int statusCode, const ServerConfig & server, const Location * location)
// {
// 	(void)statusCode;
// 	(void)server;
// 	(void)location;
// 	return ("");
// }

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

/* *************************** handler factory ****************************** */

typedef AHandler * (*HandlerFactoryFn)();
static AHandler * selectHandler(const HttpRequest& request, const Location & location)
{

	(void)location;
	// if (location.hasCgi())
	// 	return (new CgiHandler());


	static std::map<std::string, HandlerFactoryFn> handlerFactories;
	if (handlerFactories.empty()) {
		handlerFactories["GET"] = &createGetHandler;
		handlerFactories["POST"] = &createPostHandler;
		// handlerFactories["DELETE"] = &createDeleteHandler;
		}

	if (handlerFactories.find(request.getMethod()) != handlerFactories.end())
		return (handlerFactories[request.getMethod()]());
	throw HttpErrorException(405);
}

static AHandler * createGetHandler()
{
	return (new GetHandler());
}

static AHandler * createPostHandler()
{
	return (new PostHandler());
}

// static AHandler * createDeleteHandler()
// {
// 	return (new DeleteHandler());
// }

// static AHandler * createCgiHandler()
// {
// 	return (new CgiHandler());
// }
