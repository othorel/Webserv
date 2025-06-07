#include <string>
#include <unistd.h>
#include "../../../include/http/handlers/DeleteHandler.hpp"
#include "../../../include/http/HttpResponse.hpp"
#include "../../../include/http/HttpErrorException.hpp"
#include "../../../include/http/HttpUtils.hpp"

/* ************************************************************************** */
/*                                   constructors                             */
/* ************************************************************************** */

DeleteHandler::DeleteHandler()
{}

DeleteHandler::DeleteHandler(const DeleteHandler & other)
{
	(void)other;
}

/* ************************************************************************** */
/*                                    operators                               */
/* ************************************************************************** */

DeleteHandler & DeleteHandler::operator=(const DeleteHandler & other)
{
	(void)other;
	return (*this);
}

/* ************************************************************************** */
/*                                   destructors                              */
/* ************************************************************************** */

DeleteHandler::~DeleteHandler()
{}

/* ************************************************************************** */
/*                                      handler                               */
/* ************************************************************************** */

HttpResponse DeleteHandler::handle(const HttpRequest & request, const Location & location , const ServerConfig & server)
{
	std::string path = selectRoot(server, location) + request.getTarget();

	checkDeleteValidity(path);

	if (unlink(path.c_str()) == -1) {
		throw HttpErrorException(500); }

	std::map<std::string, std::string> headers;
	HttpResponse httpResponse("HTTP/1.1", 204, headers, "");
	
	return (httpResponse);
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

// 1. ✦ Rappel de la sémantique HTTP DELETE (RFC 2616 §9.7)

// The DELETE method requests that the origin server remove the resource identified by the Request-URI.
// Le client envoie une requête DELETE pour supprimer une ressource (typiquement un fichier).
// Le serveur doit supprimer cette ressource si elle existe et si cela est autorisé.
// La réponse peut être :
// 200 OK : suppression réussie, avec corps facultatif
// 204 No Content : suppression réussie, sans corps
// 202 Accepted : suppression en attente (asynchrone)
// 404 Not Found : ressource inexistante
// 403 Forbidden : pas le droit de supprimer (permissions, logique serveur)

// 2. ✦ Ce que doit faire ton DeleteHandler

// 🧱 Étapes concrètes pour Webserv :
// Résoudre le chemin système du fichier à partir de l’URI (uri → chemin absolu)
// Vérifier que :
// la ressource existe (fileExists)
// c’est bien un fichier (et pas un dossier à supprimer)
// tu as les permissions nécessaires (write ou unlink)
// Appeler unlink() pour supprimer le fichier
// Gérer les erreurs (permissions, fichier introuvable, etc.)
// Retourner une réponse :
// 204 No Content si pas de corps
// 200 OK si tu veux inclure un petit corps HTML
