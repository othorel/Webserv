#include "../../include/config/ServerConfig.hpp"

ServerConfig::ServerConfig(
    std::pair<int, std::string> listen,
    std::vector<std::string> server_names,
    std::string root,
    std::map<int, std::string> error_pages,
    std::map<std::string, Location> locations
) : _listen(listen),
    _server_names(server_names),
    _root(root),
    _error_pages(error_pages),
    _locations(locations)
{}

ServerConfig::~ServerConfig() {}

ServerConfig::ServerConfig(const ServerConfig& other) :
    _listen(other._listen),
    _server_names(other._server_names),
    _root(other._root),
    _error_pages(other._error_pages),
    _locations(other._locations)
{}

ServerConfig& ServerConfig::operator=(const ServerConfig& other) {
    if (this != &other) {
        _listen = other._listen;
        _server_names = other._server_names;
        _root = other._root;
        _error_pages = other._error_pages;
        _locations = other._locations;
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
