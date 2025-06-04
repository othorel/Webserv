#include <string>
#include "../../../include/http/handlers/GetHandler.hpp"
#include "../../../include/http/HttpResponse.hpp"
#include "../../../include/http/HttpErrorException.hpp"
#include "../../../include/http/HttpUtils.hpp"

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
	if (!HttpUtils::fileExists(path)) {
		throw HttpErrorException(404); }
	if (HttpUtils::isDirectory(path)) {
		std::string indexFile = createIndexPath(path, location);
		if (HttpUtils::fileExists(indexFile)) {
			path = indexFile; }
		else if (location.isAutoIndex()) {
			std::string body = generateAutoIndex(path, request.getTarget());
			std::map<std::string, std::string> headers;
			headers["Content-Type"] = "text/html";
			headers["Content-Length"] = HttpUtils::numberToString(body.size());
			HttpResponse httpResponse("HTTP/1.1", 200, headers, body);
			return (httpResponse); }
		else
			throw HttpErrorException(403); }
	std::string body = HttpUtils::readFile(path);
	std::map<std::string, std::string> headers;
	headers["Content-Type"] = "text/html";
	headers["Content-Length"] = HttpUtils::numberToString(body.size());
	HttpResponse httpResponse("HTTP/1.1", 200, headers, body);
	return (httpResponse);
}

/* ************************************************************************** */
/*                                     tests                                  */
/* ************************************************************************** */

// Uncomment main and includes and compile with:
// g++ HttpResponse.cpp HttpUtils.cpp ResponseBuilder.cpp handlers/AHandler.cpp handlers/GetHandler.cpp ../config/Location.cpp ../config/ServerConfig.cpp HttpRequest.cpp 
#include <iostream>
#include "../../../include/http/HttpRequest.hpp"
#include "../../../include/config/Location.hpp"
#include "../../../include/config/ServerConfig.hpp"

int main()
{
	try {
		// Simule une requête GET sur "/"
		HttpRequest request(
			"GET",
			"/", // Target
			"HTTP/1.1",
			std::map<std::string, std::string>(),
			""
		);

		// Crée un Location avec autoindex actif, root vers ./www
		Location location(
			"/Users/christophedonnat/STUDENT/Webserv",                            // path
			std::vector<std::string>(1, "GET"), // allowed methods
			"",                             // upload_path
			"www",                          // root (assure-toi que www/index.html existe)
			"",                             // index (vide pour ce test)
			"",                             // redirectPath
			0,                              // redirectCode
			false,                          // hasRedirect
			true,                           // autoindex
			std::vector<std::string>(),     // cgiExtensions
			false                           // cookiesEnabled
		);

		// Crée la map de locations
		std::map<std::string, Location> locations;
		locations["/"] = location;

		// Crée le ServerConfig
		ServerConfig serverConfig(
			std::make_pair(8080, "127.0.0.1"),           // listen
			std::vector<std::string>(1, "localhost"),    // server_names
			"www",                                       // root
			std::map<int, std::string>(),                // error_pages
			locations,                                   // locations
			1024,                                        // client_max_body_size
			"SESSIONID",                                 // sessionName
			3600,                                        // sessionTimeout
			false                                        // sessionEnable
		);

		// Appelle le handler
		GetHandler handler;
		HttpResponse response = handler.handle(request, location, serverConfig);

		// Affiche la réponse HTTP
		std::cout << "Status: " << response.getStatusCode() << std::endl;
		std::cout << "Headers:\n";
		for (std::map<std::string, std::string>::const_iterator it = response.getHeaders().begin();
			 it != response.getHeaders().end(); ++it)
			std::cout << it->first << ": " << it->second << std::endl;
		std::cout << "\nBody:\n" << response.getBody() << std::endl;
	}
	catch (const HttpErrorException &e) {
		std::cerr << "Error: " << e.getStatusCode() << " " << e.what() << std::endl;
	}
}

