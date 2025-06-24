#include "../../include/config/Location.hpp"

Location::Location() {}

Location::Location(
	std::string path,
	std::vector<std::string> methods,
	std::map<int, std::string> error_pages,
	std::string upload_path,
	std::string root,
	std::string index,
	std::string redirectPath,
	int redirectCode,
	bool hasRedirect,
	bool autoindex,
	std::vector<std::string> cgiExtensions,
	bool cookiesEnabled,
	ssize_t _client_max_body_size
) : _path(path),
	_methods(methods),
	_error_pages(error_pages),
	_upload_path(upload_path),
	_root(root),
	_index(index),
	_redirectPath(redirectPath),
	_redirectCode(redirectCode),
	_hasRedirect(hasRedirect),
	_autoindex(autoindex),
	_cgiExtensions(cgiExtensions),
	_cookiesEnabled(cookiesEnabled),
	_client_max_body_size(_client_max_body_size)
{}

Location::~Location() {}

Location::Location(const Location& other) : 
	_path(other._path),
	_methods(other._methods),
	_error_pages(other._error_pages),
	_upload_path(other._upload_path),
	_root(other._root),
	_index(other._index),
	_redirectPath(other._redirectPath),
	_redirectCode(other._redirectCode),
	_hasRedirect(other._hasRedirect),
	_autoindex(other._autoindex),
	_cgiExtensions(other._cgiExtensions),
	_cookiesEnabled(other._cookiesEnabled),
	_client_max_body_size(other._client_max_body_size)
{}

Location& Location::operator=(const Location& other) {
	if (this != &other) {
		_path = other._path;
		_methods = other._methods;
		_error_pages = other._error_pages;
		_upload_path = other._upload_path;
		_root = other._root;
		_index = other._index;
		_redirectPath = other._redirectPath;
		_redirectCode = other._redirectCode;
		_hasRedirect = other._hasRedirect;
		_autoindex = other._autoindex;
		_cgiExtensions = other._cgiExtensions;
		_cookiesEnabled = other._cookiesEnabled;
		_client_max_body_size = other._client_max_body_size;
	}
	return (*this);
}

const std::string& Location::getPath() const {
	return (_path);
}

const std::vector<std::string>& Location::getMethods() const {
	return (_methods);
}

const std::string& Location::getUploadPath() const {
	return (_upload_path);
}

const std::string& Location::getRoot() const {
	return (_root);
}

const std::string& Location::getIndex() const {
	return (_index);
}

const std::string& Location::getRedirectPath() const {
	return (_redirectPath);
}

int Location::getRedirectCode() const {
	return (_redirectCode);
}

bool Location::hasRedirect() const {
	return (_hasRedirect);
}

bool Location::isAutoIndex() const {
	return (_autoindex);
}

bool Location::isValidMethod(const std::string& method) const {
	return (std::find(_methods.begin(), _methods.end(), method) != _methods.end());
}

bool Location::hasCgi() const {
	return (!_cgiExtensions.empty());
}

const std::vector<std::string>& Location::getCgiExtensions() const {
	return (_cgiExtensions);
}

bool Location::isCookiesEnabled() const {
	return (_cookiesEnabled);
}

const std::map<int, std::string>& Location::getErrorPages() const {
    return (_error_pages);
}

bool Location::hasErrorPage(int code) const {
	return (_error_pages.find(code) != _error_pages.end());
}

const std::string& Location::getErrorPage(int code) const {
	std::map<int, std::string>::const_iterator it = _error_pages.find(code);
	if (it == _error_pages.end())
		throw std::out_of_range("No error page defined for this code");
	return (it->second);
}

ssize_t Location::getClientMaxBodySize() const {
	return (_client_max_body_size);
}
