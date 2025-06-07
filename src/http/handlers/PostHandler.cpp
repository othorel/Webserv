#include <string>
#include "../../../include/http/handlers/PostHandler.hpp"
#include "../../../include/http/HttpResponse.hpp"
#include "../../../include/http/HttpErrorException.hpp"
#include "../../../include/http/HttpUtils.hpp"

/* ************************************************************************** */
/*                                   constructors                             */
/* ************************************************************************** */

PostHandler::PostHandler()
{}

PostHandler::PostHandler(const PostHandler & other)
{
	(void)other;
}

/* ************************************************************************** */
/*                                    operators                               */
/* ************************************************************************** */

PostHandler & PostHandler::operator=(const PostHandler & other)
{
	(void)other;
	return (*this);
}

/* ************************************************************************** */
/*                                   destructors                              */
/* ************************************************************************** */

PostHandler::~PostHandler()
{}

/* ************************************************************************** */
/*                                      handler                               */
/* ************************************************************************** */

HttpResponse PostHandler::handle(
	const HttpRequest & request, const Location & location , const ServerConfig & server)
{
	std::string path = selectRoot(server, location) + location.getUploadPath();
	if (!path.empty() && path[path.size() - 1] == '/') {
		path.erase(path.size() - 1); }

	checkPostValidity(request, location, server, path);

	std::string filename = createPostFileName(request, server, path);

	try {
		HttpUtils::writeFile(filename, request.getBody(), request.getBodyLength()); }
	catch (std::exception & e) {
		throw HttpErrorException(500); }

	std::map<std::string, std::string> headers;
	std::string relativePath = location.getUploadPath();
	if (!relativePath.empty() && relativePath[relativePath.size() - 1] != '/') {
		relativePath += "/"; }
	relativePath += filename.substr(filename.find_last_of("/") + 1);
	headers["Location"] = relativePath;
	headers["Content-Type"] = "text/html";
	std::string body =
		"<html><body><h1>201 Created</h1>\n"
		"<p>The resource has been successfully created.</p>\n"
		"<a href=\"" + relativePath  + "\">See file</a>\n"
		"</body></html>";
	HttpResponse httpResponse("HTTP/1.1", 201, headers, body);

	return (httpResponse);
}

/* ************************************************************************** */
/*                            non member functions                            */
/* ************************************************************************** */

static void checkPostValidity(
	const HttpRequest & request, const Location & location ,
	const ServerConfig & server, const std::string & path)
{
	if (location.getUploadPath().empty()) {
		throw HttpErrorException(403); }
	if (request.getBodyLength() > static_cast<unsigned int>(server.getClientMaxBodySize())) {
		throw HttpErrorException(413); }
	if (!HttpUtils::fileExists(path)) {
		throw HttpErrorException(404); }
	if (!HttpUtils::isDirectory(path)) {
		throw HttpErrorException(403); }
	if (!HttpUtils::hasWritePermission(path)) {
		throw HttpErrorException(403); }
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

// Que fait une requête POST ?

// Une requête POST est utilisée pour envoyer des données au serveur dans le but :
// d'ajouter une ressource (ex. : créer un fichier, enregistrer un formulaire…)
// ou de déclencher un traitement côté serveur (ex. : script CGI).

// 🗂 Deux cas possibles pour POST dans ton serveur :

// 1. ✅ Upload direct vers un dossier
// Tu écris le corps de la requête dans un fichier (ex : /upload/form_data.txt).
// Où écrire ?
// Si location.getUploadPath() est défini → tu écris dans ce répertoire.
// Sinon, c’est une erreur 403 (forbidden) ou 501 (not implemented).
// Nom du fichier ?
// Tu peux générer un nom unique (ex: upload_001.txt).
// Ou utiliser un nom basé sur l'URI (mais attention aux conflits).
// Si le fichier existe ?
// ⚠️ Par défaut, le POST n’est pas censé écraser.
// Plusieurs options : écraser, refuser (409 Conflict), ou créer un nom alternatif.
// Création du dossier ?
// Si le répertoire upload_path n’existe pas → erreur 500 (serveur).

// 2. ⚙️ Déléguer à un script CGI
// Si la Location accepte les extensions CGI (comme .php, .py) :
// Tu exécutes un script via fork() + execve().
// Tu passes les données POST via stdin du script.
// Tu récupères la sortie du script et la retournes comme réponse HTTP.
// 🧠 Cas d’erreurs à gérer :
// Cas	Code HTTP
// Pas d’upload_path défini	403 Forbidden
// Dossier d’upload inexistant / inaccessible	500 Internal Server Error
// Fichier déjà existant	409 Conflict (ou overwrite si prévu)
// Erreur d’écriture	500 Internal Server Error

// 📝 Résumé de ce que fera PostHandler::handle :

// Vérifie que la méthode POST est autorisée.
// Vérifie que location.getUploadPath() existe et est accessible.
// Construit un chemin de fichier (upload).
// Ouvre/crée ce fichier (vérifie s’il existe déjà).
// Écrit le request.getBody() dans ce fichier.
// Crée une réponse HTTP (201 Created, 200 OK ou 409 si conflit).
