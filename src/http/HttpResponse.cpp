#include <string>
#include <map>
#include <sstream>
#include "../../include/http/HttpResponse.hpp"
#include "../../include/http/HttpUtils.hpp"

/* ************************************************************************** */
/*                                   constructors                             */
/* ************************************************************************** */

HttpResponse::HttpResponse() :
	_version("HTTP/1.1"),
	_statusCode(200),
	_headers(),
	_body()
{}

HttpResponse::HttpResponse(
		const std::string & version,
		int statusCode,
		const std::map<std::string, std::string> & headers,
		const std::string & body) :
	_version(version),
	_statusCode(statusCode),
	_headers(headers),
	_body(body)
{}
HttpResponse::HttpResponse(const HttpResponse & other) :
	_version(other._version),
	_statusCode(other._statusCode),
	_headers(other._headers),
	_body(other._body)
{}

/* ************************************************************************** */
/*                                     operators                              */
/* ************************************************************************** */

HttpResponse & HttpResponse::operator=(const HttpResponse & other)
{
	if (this != &other) {
		_version = other._version;
		_statusCode = other._statusCode;
		_headers = other._headers;
		_body = other._body;
	}
	return (*this);
}

/* ************************************************************************** */
/*                                    destructor                              */
/* ************************************************************************** */

HttpResponse::~HttpResponse()
{}

/* ************************************************************************** */
/*                                      getters                               */
/* ************************************************************************** */

const std::string & HttpResponse::getVersion() const
{
	return (_version);
}

int HttpResponse::getStatusCode() const
{
	return (_statusCode);
}

const std::map<std::string, std::string> & HttpResponse::getHeaders() const
{
	return (_headers);
}

const std::string & HttpResponse::getBody() const
{
	return (_body);
}

/* ************************************************************************** */
/*                                     setters                                */
/* ************************************************************************** */

void HttpResponse::addHeader(const std::string & key, const std::string & value)
{
	_headers[key] = value;
}

/* ************************************************************************** */
/*                               to raw string                                */
/* ************************************************************************** */

std::string HttpResponse::toRawString() const
{
	std::ostringstream oss;
	oss << _version << " " << _statusCode << " "
		<< HttpUtils::httpStatusMessage(_statusCode) << "\r\n";
	std::map<std::string, std::string>::const_iterator cit = _headers.begin();
	for (; cit != _headers.end(); cit++)
		oss << cit->first << ": " << cit->second << "\r\n";
	oss << "\r\n" << _body;
	return (oss.str());
}
