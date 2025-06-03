#include <string>
#include <sstream>
#include <sys/stat.h>
#include <dirent.h>

#include "../../include/http/ResponseBuilder.hpp"
#include "../../include/http/HttpRequest.hpp"
#include "../../include/http/HttpResponse.hpp"
#include "../../include/config/Location.hpp"

std::string httpStatusMessage(int code);

ResponseBuilder::ResponseBuilder() :
	_httpResponse()
{}

ResponseBuilder::ResponseBuilder(
		const HttpRequest& request,
		const std::map<std::string, Location> & routes)
{
	_httpResponse = buildResponse(request, routes);
}

ResponseBuilder::ResponseBuilder(const ResponseBuilder & other) :
	_httpResponse(other._httpResponse)
{}

ResponseBuilder & ResponseBuilder::operator=(const ResponseBuilder & other)
{
	if (&other != this) {
		this->_httpResponse = other._httpResponse;
	}
	return (*this);
}

ResponseBuilder::~ResponseBuilder()
{}

const Location & ResponseBuilder::findMatchingRoute(
		const std::map<std::string, Location> & routes,
		const std::string & target) const
{
	std::string bestMatch = "";
	std::map<std::string, Location>::const_iterator it;
	for (it = routes.begin(); it != routes.end(); ++it)
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
	return (routes.find(bestMatch)->second);
}

void ResponseBuilder::buildRedirect(int code, const std::string & path)
{
	std::string body = "<html><body><h1>" + httpStatusMessage(code) + "</h1>"
        "<p>Redirecting to <a href=\"" + path + "\">" + path + "</a></p></body></html>";
	std::map<std::string, std::string> headers;
	headers["Location"] = path;
	headers["Content-Type"] = "text/html";
	headers["Content-Length"] = intToString(body);
	_httpResponse = HttpResponse("HTTP/1.1", code, headers, body);
}

std::string HttpResponse::generateAutoIndex(const std::string & dirPath, const std::string & uriPath)
{
	DIR* dir = opendir(dirPath.c_str());
	if (!dir)
		throw HttpErrorException(500);

	std::ostringstream html;
	html << "<html><body><h1>Index of " << uriPath << "</h1><ul>";

	struct dirent* entry;
	while ((entry = readdir(dir)) != NULL)
	{
		std::string name = entry->d_name;
		if (name == "." || name == "..")
			continue;

		html << "<li><a href=\"" << uriPath;
		if (!uriPath.empty() && uriPath[uriPath.size() - 1] != '/')
			html << "/";
		html << name << "\">" << name << "</a></li>";
	}
	closedir(dir);
	html << "</ul></body></html>";
	return (html.str());
}

const HttpResponse & ResponseBuilder::buildResponse(
		const HttpRequest& request,
		const std::map<std::string, Location> & routes)
{
	try {
		// Find location corresponding to target
		const Location route = findMatchingRoute(routes, request.getTarget());
		// Check if method is authorised
		std::string method = request.getMethod();
		if (!route.isValidMethod(method))
			throw HttpErrorException(405);
		// Calculate Path
		std::string path = route.getRoot() + request.getTarget().substr(route.getPath().length());
		// Redirection ?
		if (route.hasRedirect()) {
			buildRedirect(route.getRedirectCode(), route.getRedirectPath());
			return (_httpResponse);
		}
		// path is directory and autoindex
		if (isDirectory(path)) {
			std::string indexFile = path + "/index.html";
			if (fileExists(indexFile))
				path = indexFile;
			else if (route.isAutoIndex()) {
				std::string body = generateAutoIndex(path, request.getTarget());
				std::map<std::string, std::string> headers;
				headers["Content-Type"] = "text/html";
				headers["Content-Length"] = intToString(body);
				_httpResponse = HttpResponse("HTTP/1.1", 200, headers, body);
				return (_httpResponse);
			}
			else
				throw HttpErrorException(403);
		}
		// 5. Gérer le CGI (si activé dans la location)
		// Si location.getCgiPath() est non vide et que le fichier est un CGI (ex: .py, .php) :
		// fork()
		// execve() du script
		// Récupérer la sortie via pipe
		// En faire le corps de la HttpResponse
		if (route.hasCgi()) {
			handleCgi();
		}



		// 6. Lire le fichier demandé
		// Ouvrir le fichier en lecture binaire
		// Lire son contenu
		// Déduire le type MIME depuis l’extension (ex: .html, .jpg, etc.)
		// Remplir :
		// _statusCode = 200
		// _headers["Content-Type"] = "text/html" (ou autre)
		// _headers["Content-Length"] = taille du corps
		// _body = contenu du fichier

		// 7. Créer la réponse HTTP
		// Construire l’objet :
		// HttpResponse response("HTTP/1.1", 200, headers, body);
		// Appeler response.toRawString() pour envoyer la réponse finale au client
	}
	catch (const HttpErrorException & e) {
		buildError(e.getStatusCode());
	}
	_httpResponse = HttpResponse("HTTP/1.1", code, headers, body);
	return (_httpResponse);
}

std::string HttpResponse::intToString(const std::string & body)
{
	std::ostringstream oss;
	oss << body.size();
	return (oss.str());
}

bool HttpResponse::isDirectory(const std::string & path)
{
	struct stat s;
	if (!stat(path.c_str(), &s))
		return ((s.st_mode & S_IFMT) == S_IFDIR);
	return (false);
}

bool HttpResponse::fileExists(const std::string & path)
{
	struct stat s;
	return (stat(path.c_str(), &s) == 0 && S_ISREG(s.st_mode));
}
