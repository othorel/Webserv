#include <iostream>
#include <string>
#include <map>
#include "HttpRequest.hpp"

HttpRequest::HttpRequest() :
	_method(),
	_uri(),
	_version(),
	_headers(),
	_body(),
	_contentLength(0)
{}

HttpRequest::HttpRequest(const std::string & method,
						const std::string & uri,
						const std::string & version,
						const std::map<std::string, std::string> & headers,
						const std::string & body) :
	_method(method),
	_uri(uri),
	_version(version),
	_headers(headers),
	_body(body),
	_contentLength(0)
{
	if (hasHeader("content-length"))
		_contentLength = std::atoi(getHeaderValue("content-length").c_str());
}

HttpRequest::~HttpRequest() {}
 
HttpRequest::HttpRequest(const HttpRequest & other) :
	_method(other._method),
	_uri(other._uri),
	_version(other._version),
	_headers(other._headers),
	_body(other._body),
	_contentLength(other._contentLength)
{}

HttpRequest & HttpRequest::operator=(const HttpRequest & other)
{
	if (this != &other) {
		_method = other._method;
		_uri = other._uri;
		_version = other._version;
		_headers = other._headers;
		_body = other._body;
		_contentLength = other._contentLength;
	}
	return (*this);
}

const std::string & HttpRequest::getMethod() const
{
	return (_method);
}

const std::string & HttpRequest::getUri() const
{
	return (_uri);
}

const std::string & HttpRequest::getVersion() const
{
	return (_version);
}

const std::string & HttpRequest::getBody() const
{
	return (_body);
}

const std::map<std::string, std::string>  & HttpRequest::getHeaders() const
{
	return (_headers);
}

std::string HttpRequest::getHeaderValue(const std::string & key) const
{
	std::map<std::string, std::string>::const_iterator cit = _headers.find(key);
	if (cit == _headers.end())
		return ("");
	return (cit->second);
}

bool HttpRequest::hasHeader(const std::string & key) const
{
	std::map<std::string, std::string>::const_iterator cit = _headers.find(key);
	return (cit != _headers.end());
}

size_t HttpRequest::getBodyLength() const
{
	return (_contentLength);
}

void HttpRequest::debug() const
{
	std::cout << "\n______ HttpRequest Debug______" << std::endl;

	std::cout	<< "Method : " << _method << "\n"
				<< "Uri : " << _uri << "\n"
				<< "Http version : " << _version << std::endl;

	std::map<std::string, std::string>::const_iterator cit = _headers.begin();
	for (; cit != _headers.end(); cit++) {
		std::cout << cit->first << " : " << cit->second << std::endl;
	}
	
	std::cout	<< "Body : " << _body << std::endl;
	std::cout << "______________________________\n" << std::endl;
}
