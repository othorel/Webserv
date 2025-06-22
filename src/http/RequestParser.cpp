#include <iostream>
#include <sstream>
#include <string>
#include <map>
#include <exception>
#include "../../include/http/HttpErrorException.hpp"
#include "../../include/http/RequestParser.hpp"

static std::string	extractLineAndRemove(std::string & input);
static std::string extractMethod(std::istringstream & iss);
static std::string extractUri(std::istringstream & iss);
static std::string extractVersion(std::istringstream & iss);
static std::map<std::string, std::string> extractHeaders(std::string & buffer);
static size_t calculateContentLength(const std::map<std::string, std::string> & headers);
static std::string extractBody(std::string & buffer, int contentLength);

/* ************************************************************************** */
/*                                constructors                                */
/* ************************************************************************** */

RequestParser::RequestParser() :
	_httpRequest(NULL)
{}

// Constructor that parses a raw HTTP request string
// and dynamically allocates a new HttpRequest object.
// Throws HttpErrorException if the request is invalid.
RequestParser::RequestParser(const std::string & raw_request) :
	_httpRequest(NULL)
{
	parseRequest(raw_request);
}

/* ************************************************************************** */
/*                                 destructor                                 */
/* ************************************************************************** */

RequestParser::~RequestParser()
{
	delete _httpRequest;
}

/* ************************************************************************** */
/*                                   getters                                  */
/* ************************************************************************** */

const HttpRequest & RequestParser::getHttpRequest() const
{
	if (!_httpRequest)
		throw HttpErrorException(500, "in HR: Request is NULL.");
	return (*_httpRequest);
}

/* ************************************************************************** */
/*                                   parser                                   */
/* ************************************************************************** */

void RequestParser::parseRequest(const std::string & raw_request)
{
	if (_httpRequest)
		delete _httpRequest;

	if (raw_request.empty())
		throw HttpErrorException(400, "in HR: Raw request is empty.");

	std::string buffer = raw_request;
	std::string requestLine = extractLineAndRemove(buffer);

	if (requestLine.empty())
		throw HttpErrorException(400, "in HR: Requestline is empty.");
	if (buffer.empty())
		throw HttpErrorException(400, "in HR: Buffer is empty.");

	std::istringstream iss(requestLine);
	std::string method = extractMethod(iss);
	std::string uri = extractUri(iss);
	std::string version = extractVersion(iss);
	
	if (version != "HTTP/1.1")
		throw HttpErrorException(400, "in HR: Invalid HTTP version.");

	std::string shouldBeEmpty;
	if ((iss >> shouldBeEmpty))
		throw HttpErrorException(400, "in HR: Too many arguments in request line.");

	std::map<std::string, std::string> headers = extractHeaders(buffer);
	if (!headers.count("host") || headers.find("host")->second.empty())
		throw HttpErrorException(400, "in HR: No host header.");

	unsigned int contentLength = calculateContentLength(headers);
	std::string body = extractBody(buffer, contentLength);

	try {
		_httpRequest = new HttpRequest(method, uri, version, headers, body); }
	catch (const std::bad_alloc&) {
		throw HttpErrorException(500, "in HR: Bad alloc."); }
}

/* ************************************************************************** */
/*                             ownership transfer                             */
/* ************************************************************************** */

// Transfers ownership of the internal HttpRequest pointer to the caller.
// After this call, the RequestParser no longer manages or deletes the object.
// Returns a pointer to the HttpRequest, or NULL if it was already released.
HttpRequest * RequestParser::release()
{
	HttpRequest * temp = _httpRequest;
	_httpRequest = NULL;
	return (temp);
}

/* ************************************************************************** */
/*                            non member functions                            */
/* ************************************************************************** */

std::string extractMethod(std::istringstream & iss)
{
	std::string method;
	if (!(iss >> method))
		throw HttpErrorException(400, "in HR: Method is empty.");
	return (method);
}

std::string extractUri(std::istringstream & iss)
{
	std::string uri;
	if (!(iss >> uri))
		throw HttpErrorException(400, "in HR: Uri is empty.");
	return (uri);
}

std::string extractVersion(std::istringstream & iss)
{
	std::string version;
	if (!(iss >> version))
		throw HttpErrorException(400, "in HR: Version is empty.");
	return (version);
}

std::map<std::string, std::string> extractHeaders(std::string & buffer)
{
	if (buffer.empty())
		throw HttpErrorException(400, "in HR: Buffer is empty.");

	std::map<std::string, std::string> headers;
	std::string headerLine;

	while ((headerLine = extractLineAndRemove(buffer)) != "") {
		size_t pos = headerLine.find(':');
		if (pos == std::string::npos)
			throw HttpErrorException(400, "in HR: No semicolon in header line.");

		std::string key = headerLine.substr(0, pos);
		std::string value = headerLine.substr(pos + 1);

		for (size_t i = 0; i < key.length(); i++)
			key[i] = std::tolower(key[i]);
		pos = value.find_first_not_of(" \t");
		if (pos == std::string::npos)
			throw HttpErrorException(400, "in HR: No whitespace in header.");
		value = value.substr(pos);
		size_t end = value.find_last_not_of(" \t");
		if (end != std::string::npos)
			value = value.substr(0, end + 1);

		headers[key] = value;
	}

	return (headers);
}

size_t calculateContentLength(const std::map<std::string, std::string> & headers)
{
	size_t contentLength = 0;
	std::map<std::string, std::string>::const_iterator cit = headers.find("content-length");
	if (cit != headers.end()) {
		std::string contentLengthValue = cit->second;
		if (contentLengthValue.empty())
			throw HttpErrorException(400, "in HR: Content-Length is empty.");
		for (size_t i = 0; i < contentLengthValue.length(); i++) {
			if (!std::isdigit(contentLengthValue[i]))
				throw HttpErrorException(400, "in HR: Non digit character in Content-Length.");
		}
		std::istringstream iss(contentLengthValue);
		iss >> contentLength;
	}
	return (contentLength);
}

std::string extractBody(std::string & buffer, int contentLength)
{
	std::string body;

	if (buffer.empty() || contentLength == 0) {
		return (body); }

	size_t dataSize = buffer.length() < static_cast<size_t>(contentLength) ? buffer.length() : static_cast<size_t>(contentLength);
	body = buffer.substr(0, dataSize);
	buffer.erase(0, dataSize);

	return (body);
}

// Function that returns a substring from the beginning to the first '\n' found
// (removing the '\r' before the '\n' if any)
// and truncates the input by removing the substring (and the '\n') from the it.
// Returns an empty string if the input is empty.
// Returns a copy of the whole input if there is no '\n', and earases the input.
std::string extractLineAndRemove(std::string & input)
{
	if (input.empty())
		return ("");

	size_t pos = input.find('\n');
	std::string	line;

	if (pos != std::string::npos) {
		line = input.substr(0, pos);
		if (!line.empty() && line[line.size() - 1] == '\r')
			line.resize(line.size() - 1);
		input.erase(0, pos + 1);
	}
	else {
		line = input;
		input.clear();
	}
	return (line);
}
