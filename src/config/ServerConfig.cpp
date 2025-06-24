#include "../../include/config/ServerConfig.hpp"

ServerConfig::ServerConfig() :
	_listen(),
	_server_names(),
	_root(),
	_error_pages(),
	_locations(),
	_client_max_body_size(),
	_keepAlive(),
	_keepAliveTimeout(),
	_keepAliveMaxRequests(),
	_sessionName(),
	_sessionTimeout(),
	_sessionEnable()
{}

ServerConfig::ServerConfig(
	std::pair<int, std::string> listen,
	std::vector<std::string> server_names,
	std::string root,
	std::map<int, std::string> error_pages,
	std::map<std::string, Location> locations,
	ssize_t client_max_body_size,
	bool keepAlive,
	int keepAliveTimeout,
	int keepAliveMaxRequests,
	std::string sessionName,
	int sessionTimeout,
	bool sessionEnable
) : _listen(listen),
	_server_names(server_names),
	_root(root),
	_error_pages(error_pages),
	_locations(locations),
	_client_max_body_size(client_max_body_size),
	_keepAlive(keepAlive),
	_keepAliveTimeout(keepAliveTimeout),
	_keepAliveMaxRequests(keepAliveMaxRequests),
	_sessionName(sessionName),
	_sessionTimeout(sessionTimeout),
	_sessionEnable(sessionEnable)
{}

ServerConfig::~ServerConfig() {}

ServerConfig::ServerConfig(const ServerConfig& other) :
	_listen(other._listen),
	_server_names(other._server_names),
	_root(other._root),
	_error_pages(other._error_pages),
	_locations(other._locations),
	_client_max_body_size(other._client_max_body_size),
	_keepAlive(other._keepAlive),
	_keepAliveTimeout(other._keepAliveTimeout),
	_keepAliveMaxRequests(other._keepAliveMaxRequests),
	_sessionName(other._sessionName),
	_sessionTimeout(other._sessionTimeout),
	_sessionEnable(other._sessionEnable)
{}

ServerConfig& ServerConfig::operator=(const ServerConfig& other) {
	if (this != &other) {
		_listen = other._listen;
		_server_names = other._server_names;
		_root = other._root;
		_error_pages = other._error_pages;
		_locations = other._locations;
		_client_max_body_size = other._client_max_body_size;
		_keepAlive = other._keepAlive,
		_keepAliveTimeout = other._keepAliveTimeout;
		_keepAliveMaxRequests = other._keepAliveMaxRequests;
		_sessionName = other._sessionName;
		_sessionTimeout = other._sessionTimeout;
		_sessionEnable = other._sessionEnable;
	}
	return (*this);
}

const std::pair<int, std::string>& ServerConfig::getListen() const {
	return (_listen);
}

const std::vector<std::string>& ServerConfig::getServerNames() const {
	return (_server_names);
}

const std::string& ServerConfig::getRoot() const {
	return (_root);
}

const std::map<int, std::string>& ServerConfig::getErrorPages() const {
	return (_error_pages);
}

const std::map<std::string, Location>& ServerConfig::getLocations() const {
	return (_locations);
}

ssize_t ServerConfig::getClientMaxBodySize() const {
	return (_client_max_body_size);
}

bool ServerConfig::getKeepAlive() const {
	return (_keepAlive);
}

int ServerConfig::getKeepAliveTimeout() const {
	return (_keepAliveTimeout);
}

int ServerConfig::getKeepAliveMaxRequests() const {
	return (_keepAliveMaxRequests);
}

const std::string& ServerConfig::getSessionName() const {
	return (_sessionName);
}

int ServerConfig::getSessionTimeout() const {
	return (_sessionTimeout);
}

bool ServerConfig::isSessionEnable() const {
	return (_sessionEnable);
}

bool ServerConfig::hasErrorPage(int code) const {
	return (_error_pages.find(code) != _error_pages.end());
}

const std::string& ServerConfig::getErrorPage(int code) const {
	std::map<int, std::string>::const_iterator it = _error_pages.find(code);
	if (it == _error_pages.end())
		throw std::out_of_range("No error page defined for this code");
	return (it->second);
}

bool ServerConfig::hasServerName(const std::string & nameToFind) const
{
	return std::find(_server_names.begin(), _server_names.end(), nameToFind) != _server_names.end();
}

void ServerConfig::printServerConfig(int index) const {
	std::cout << "Server " << (index + 1) << ":" << std::endl;
	std::cout << std::endl;
	std::cout << "  IP       : " << this->getListen().second << std::endl;
	std::cout << "  Port     : " << this->getListen().first << std::endl;
	std::cout << "  Root     : " << this->getRoot() << std::endl;
	if (this->getServerNames().empty()) {
		std::cerr << "No server_name defined (this server will act as the default)." << std::endl;
		std::cout << "  Server names: " << std::endl;
	} else {
		std::cout << "  Server names: ";
		for (size_t j = 0; j < this->getServerNames().size(); ++j)
			std::cout << this->getServerNames()[j] << " ";
		std::cout << std::endl;
	}
	std::cout << "  Max client body size: " << this->getClientMaxBodySize() << " bytes" << std::endl;
	std::cout << "  Session:" << std::endl;
	std::cout << "    Enabled : " << (this->isSessionEnable() ? "yes" : "no") << std::endl;
	std::cout << "    Name    : " << this->getSessionName() << std::endl;
	std::cout << "    Timeout : " << this->getSessionTimeout() << " seconds" << std::endl;
	std::cout << "  Error pages:" << std::endl;
	const std::map<int, std::string>& errors = this->getErrorPages();
	for (std::map<int, std::string>::const_iterator eit = errors.begin(); eit != errors.end(); ++eit)
		std::cout << "    " << eit->first << " => " << eit->second << std::endl;
	// Locations
	std::cout << std::endl;
	std::cout << "  Locations:" << std::endl;
	std::cout << std::endl;
	const std::map<std::string, Location>& locs = this->getLocations();
	for (std::map<std::string, Location>::const_iterator it = locs.begin(); it != locs.end(); ++it) {
		const Location& loc = it->second;
		std::cout << "    Path: " << it->first << std::endl;
		std::cout << "      Root        : " << loc.getRoot() << std::endl;
		std::cout << "      Index       : " << loc.getIndex() << std::endl;
		std::cout << "      Autoindex   : " << (loc.isAutoIndex() ? "on" : "off") << std::endl;
		std::cout << "      Allowed methods: ";
		for (size_t k = 0; k < loc.getMethods().size(); ++k)
			std::cout << loc.getMethods()[k] << " ";
		std::cout << std::endl;
		std::cout << "      Upload path : " << loc.getUploadPath() << std::endl;
		std::cout << "      CGI extensions: ";
		const std::vector<std::string>& cgiExts = loc.getCgiExtensions();
		for (size_t k = 0; k < cgiExts.size(); ++k)
			std::cout << cgiExts[k] << " ";
		std::cout << std::endl;
		std::cout << "      Cookies     : " << (loc.isCookiesEnabled() ? "enabled" : "disabled") << std::endl;

		if (loc.hasRedirect())
			std::cout << "      Redirect    : " << loc.getRedirectCode() << " => " << loc.getRedirectPath() << std::endl;
	}
	std::cout << std::endl;
}
