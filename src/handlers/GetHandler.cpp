#include "../../include/handlers/GetHandler.hpp"


		std::string path = location.getRoot() + request.getTarget().substr(location.getPath().length());
		// path is directory and autoindex
		if (isDirectory(path)) {
			std::string indexFile = path + "/index.html";
			if (fileExists(indexFile))
				path = indexFile;
			else if (location.isAutoIndex()) {
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
		if (location.hasCgi()) {
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



std::string ResponseBuilder::generateAutoIndex(const std::string & dirPath, const std::string & uriPath)
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