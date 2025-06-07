#include <iostream>
#include <string>
#include <map>
#include "../../include/http/HttpRequest.hpp"
#include "../../include/http/HttpUtils.hpp"

/* ************************************************************************** */
/*                                   constructors                             */
/* ************************************************************************** */

HttpRequest::HttpRequest() :
	_method(),
	_target(),
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
	_target(uri),
	_version(version),
	_headers(headers),
	_body(body),
	_contentLength(0)
{
	if (hasHeader("content-length"))
		_contentLength = HttpUtils::stringToInt(getHeaderValue("content-length").c_str());
}

HttpRequest::HttpRequest(const HttpRequest & other) :
	_method(other._method),
	_target(other._target),
	_version(other._version),
	_headers(other._headers),
	_body(other._body),
	_contentLength(other._contentLength)
{}

/* ************************************************************************** */
/*                                    operators                               */
/* ************************************************************************** */

HttpRequest & HttpRequest::operator=(const HttpRequest & other)
{
	if (this != &other) {
		_method = other._method;
		_target = other._target;
		_version = other._version;
		_headers = other._headers;
		_body = other._body;
		_contentLength = other._contentLength;
	}
	return (*this);
}

/* ************************************************************************** */
/*                                   destructor                               */
/* ************************************************************************** */

HttpRequest::~HttpRequest() {}

/* ************************************************************************** */
/*                                    getters                                 */
/* ************************************************************************** */

const std::string & HttpRequest::getMethod() const
{
	return (_method);
}

const std::string & HttpRequest::getTarget() const
{
	return (_target);
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

size_t HttpRequest::getContentLength() const
{
	return (_contentLength);
}

size_t HttpRequest::getCurrentBodyLength() const
{
	return (_body.size());
}

size_t HttpRequest::getMissingBodyLength() const
{
	return (_body.size() >= _contentLength ? 0 : _contentLength - _body.size());
}

bool HttpRequest::BodyIsComplete() const
{
	return (_body.size() >= _contentLength);
}

/* ************************************************************************** */
/*                                    setters                                 */
/* ************************************************************************** */

size_t HttpRequest::AppendBody(const std::string & buffer)
{
	_body += buffer;
	return (getMissingBodyLength());
}

/* ************************************************************************** */
/*                            other public methods                            */
/* ************************************************************************** */

bool HttpRequest::hasHeader(const std::string & key) const
{
	std::map<std::string, std::string>::const_iterator cit = _headers.find(key);
	return (cit != _headers.end());
}

void HttpRequest::debug() const
{
	std::cout << "\n______ HttpRequest Debug______" << std::endl;

	std::cout	<< "Method : " << _method << "\n"
				<< "Uri : " << _target << "\n"
				<< "Http version : " << _version << std::endl;

	std::map<std::string, std::string>::const_iterator cit = _headers.begin();
	for (; cit != _headers.end(); cit++) {
		std::cout << cit->first << " : " << cit->second << std::endl;
	}
	
	std::cout	<< "Body : " << _body << std::endl;
	std::cout << "______________________________\n" << std::endl;
}
