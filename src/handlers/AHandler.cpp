#include <sstream>
#include <fstream>
#include <dirent.h>
#include <sys/stat.h>
#include "../../include/handlers/AHandler.hpp"

AHandler::~AHandler()
{}
std::string httpStatusMessage(int code);

std::string AHandler::numberToString(size_t value)
{
	std::ostringstream oss;
	oss << value;
	return (oss.str());
}

std::string AHandler::numberToString(int value)
{
	std::ostringstream oss;
	oss << value;
	return (oss.str());
}

std::string AHandler::readFile(const std::string & path)
{
	std::ifstream file(path.c_str());
	if (!file) {
		throw std::runtime_error("Could not open file " + path); }
	std::ostringstream content;
	content << file.rdbuf();
	file.close();
	return (content.str());
}

bool AHandler::isDirectory(const std::string & path)
{
	struct stat s;
	if (!stat(path.c_str(), &s))
		return ((s.st_mode & S_IFMT) == S_IFDIR);
	return (false);
}

bool AHandler::fileExists(const std::string & path)
{
	struct stat s;
	return (stat(path.c_str(), &s) == 0 && S_ISREG(s.st_mode));
}

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
