#include <sstream>
#include <fstream>
#include <dirent.h>
#include <sys/stat.h>
#include "../../../include/http/handlers/AHandler.hpp"
#include "../../../include/http/HttpErrorException.hpp"

/* ************************************************************************** */
/*                                  destructor                                */
/* ************************************************************************** */

AHandler::~AHandler()
{}

/* ************************************************************************** */
/*                             protected methods                              */
/* ************************************************************************** */

std::string AHandler::createIndexPath(std::string path, const Location & location)
{
	if (!path.empty() && path.back() == '/') {
		path.pop_back(); }
	std::string indexFile = location.getIndex();
	if (indexFile.empty()) {
		indexFile = "/index.html"; }
	return (path + indexFile);
}

std::string AHandler::generateAutoIndex(const std::string & dirPath, const std::string & uriPath)
{
	DIR* dir = opendir(dirPath.c_str());
	if (!dir) {
		throw HttpErrorException(500); }
	std::ostringstream html;
	html << "<html><body><h1>Index of " << uriPath << "</h1><ul>";
	struct dirent* entry;
	while ((entry = readdir(dir)) != NULL) {
		std::string name = entry->d_name;
		if (name == "." || name == "..") {
			continue ; }
		html << "<li><a href=\"" << uriPath;
		if (!uriPath.empty() && uriPath[uriPath.size() - 1] != '/')
			html << "/";
		html << name << "\">" << name << "</a></li>"; }
	closedir(dir);
	html << "</ul></body></html>";
	return (html.str());
}

const std::string & AHandler::selectRoot(const ServerConfig & server, const Location & location)
{
	return (!location.getRoot().empty() ? location.getRoot() : server.getRoot());
}
