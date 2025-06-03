#include "../../include/config/Location.hpp"

Location::Location() {}

Location::Location(
	std::string path,
	std::vector<std::string> methods,
	std::string upload_path,
	std::string cgi_extension,
	std::string root,
	std::string index,
	bool autoindex
) : _path(path),
	_methods(methods),
	_upload_path(upload_path),
	_cgi_extension(cgi_extension),
	_root(root),
	_index(index),
	_autoindex(autoindex)
{}

Location::~Location() {}

Location::Location(const Location& other) : 
	_path(other._path),
	_methods(other._methods),
	_upload_path(other._upload_path),
	_cgi_extension(other._cgi_extension),
	_root(other._root),
	_index(other._index),
	_autoindex(other._autoindex)
{}

Location& Location::operator=(const Location& other) {
	if (this != &other) {
		_path = other._path;
		_methods = other._methods;
		_upload_path = other._upload_path;
		_cgi_extension = other._cgi_extension;
		_root = other._root;
		_index = other._index;
		_autoindex = other._autoindex;
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

const std::string& Location::getCgiExtension() const {
	return (_cgi_extension);
}

const std::string& Location::getRoot() const {
	return (_root);
}

const std::string& Location::getIndex() const {
	return (_index);
}

bool Location::isAutoIndex() const {
	return (_autoindex);
}

bool Location::isValidMethod(const std::string& method) const {
	return (std::find(_methods.begin(), _methods.end(), method) != _methods.end());
}
