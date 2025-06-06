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

//debug
# include <iostream>


HttpResponse GetHandler::handle(const HttpRequest & request, const Location & location , const ServerConfig & server)
{
	std::string path = selectRoot(server, location) + request.getTarget();

	if (DEBUG)
	{
		std::cout << "In handle" << std::endl;
		std::cout << "SelectRoot : " << selectRoot(server, location) << std::endl;
		std::cout << "requestTarget : " << request.getTarget() << std::endl;
		std::cout << "Path : " << path << std::endl << std::endl;
	}

	if (!HttpUtils::fileExists(path)) {
		if (DEBUG)
			std::cout << "In handle : file does not exist" << std::endl;
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
// g++ src/http/HttpResponse.cpp src/http/HttpUtils.cpp src/http/ResponseBuilder.cpp src/http/handlers/AHandler.cpp src/http/handlers/GetHandler.cpp src/config/Location.cpp src/config/ServerConfig.cpp src/http/HttpRequest.cpp 
#include <iostream>
#include <map>
#include "../../../include/http/HttpRequest.hpp"
#include "../../../include/http/ResponseBuilder.hpp"
#include "../../../include/config/Location.hpp"
#include "../../../include/config/ServerConfig.hpp"

int main()
{
	try {
		// Simule une requête GET sur "/"
		HttpRequest request(
			"GET",
			"/index.html", // Target
			"HTTP/1.1",
			std::map<std::string, std::string>(),
			""
		);

		std::cout << "In main : after HttpRequest." << std::endl;
		
		std::map<int, std::string> error_pages;


		// Crée un Location avec autoindex actif, root vers ./www
		Location location(
			"/Users/christophedonnat/STUDENT/Webserv",                            // path
			std::vector<std::string>(1, "GET"), // allowed methods
			error_pages,
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

			// std::string path,
			// std::vector<std::string> methods,
			// std::map<int, std::string> error_pages,
			// std::string upload_path,
			// std::string root,
			// std::string index,
			// std::string redirectPath,
			// int redirectCode,
			// bool hasRedirect,
			// bool autoindex,
			// std::vector<std::string> cgiExtensions,
			// bool cookiesEnabled

		std::cout << "In main : after Location." << std::endl;

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

		// Cree le vector de serverConfig
		std::vector<ServerConfig> serverConfigVector;
		serverConfigVector.push_back(serverConfig);

		std::cout << "In main : after ServerConfig." << std::endl;

		// Appelle Response Builder
		ResponseBuilder response(request, serverConfigVector);

		std::cout << "In main : after responseBuilder\n" << std::endl;

		std::cout << response.getHttpResponse().toRawString() << std::endl;

	}
	catch (const HttpErrorException &e) {
		std::cerr << "Error: " << e.getStatusCode() << " " << e.what() << std::endl;
	}
}

