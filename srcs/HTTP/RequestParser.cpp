#include <iostream>
#include <sstream>
#include <string>
#include <map>
#include <exception>
#include "RequestParser.hpp"

// Method that creates an HttpRequest object by parsing the whole request
// throwing invalidRequestException in case of error
RequestParser::RequestParser(const std::string & raw_request)
{
	parseRequest(raw_request);
}

RequestParser::RequestParser(const RequestParser & other) :
	_httpRequest(other._httpRequest)
{}

RequestParser & RequestParser::operator=(const RequestParser & other)
{
	if (this != &other) {
		_httpRequest = other._httpRequest;
	}
	return (*this);
}

RequestParser::~RequestParser()
{}

const HttpRequest & RequestParser::getHttpRequest() const
{
	return (_httpRequest);
}

void RequestParser::parseRequest(const std::string & raw_request)
{
	if (raw_request.empty())
		throw InvalidRequestException(400);

	std::string buffer = raw_request;

	std::string requestLine = extractLineAndRemove(buffer);
	if (requestLine.empty())
		throw InvalidRequestException(400);
	if (buffer.empty())
		throw InvalidRequestException(400);

	std::istringstream iss(requestLine);
	std::string method = extractMethod(iss);
	std::string uri = extractUri(iss);
	std::string version = extractVersion(iss);
	if (version != "HTTP/1.1")
		throw InvalidRequestException(400);
	std::string shouldBeEmpty;
	if ((iss >> shouldBeEmpty))
		throw InvalidRequestException(400);
	std::map<std::string, std::string> headers = extractHeaders(buffer);
	if (!headers.count("host") || headers.find("host")->second.empty())
		throw InvalidRequestException(400);
	unsigned int contentLength = calculateContentLength(headers);
	std::string body = extractBody(buffer, contentLength);

	_httpRequest = HttpRequest(method, uri, version, headers, body);
}

std::string RequestParser::extractMethod(std::istringstream & iss)
{
	std::string method;
	if (!(iss >> method))
		throw InvalidRequestException(400);
	return (method);
}

std::string RequestParser::extractUri(std::istringstream & iss)
{
	std::string uri;
	if (!(iss >> uri))
		throw InvalidRequestException(400);
	return (uri);
}

std::string RequestParser::extractVersion(std::istringstream & iss)
{
	std::string version;
	if (!(iss >> version))
		throw InvalidRequestException(400);
	return (version);
}

std::map<std::string, std::string> RequestParser::extractHeaders(std::string & buffer)
{
	if (buffer.empty())
		throw InvalidRequestException(400);

	std::map<std::string, std::string> headers;
	std::string headerLine;

	while ((headerLine = extractLineAndRemove(buffer)) != "") {
		size_t pos = headerLine.find(':');
		if (pos == std::string::npos)
			throw InvalidRequestException(400);

		std::string key = headerLine.substr(0, pos);
		std::string value = headerLine.substr(pos + 1);

		for (size_t i = 0; i < key.length(); i++)
			key[i] = std::tolower(key[i]);
		pos = value.find_first_not_of(" \t");
		if (pos == std::string::npos)
			throw InvalidRequestException(400);
		value = value.substr(pos);
		size_t end = value.find_last_not_of(" \t");
		if (end != std::string::npos)
			value = value.substr(0, end + 1);

		headers[key] = value;
	}

	return (headers);
}

int RequestParser::calculateContentLength(const std::map<std::string, std::string> & headers)
{
	int contentLength = 0;
	std::map<std::string, std::string>::const_iterator cit = headers.find("content-length");
	if (cit != headers.end()) {
		std::string contentLengthValue = cit->second;
		if (contentLengthValue.empty())
			throw InvalidRequestException(400);
		for (size_t i = 0; i < contentLengthValue.length(); i++) {
			if (!std::isdigit(contentLengthValue[i]))
				throw InvalidRequestException(400);
		}
		std::istringstream iss(contentLengthValue);
		iss >> contentLength;
	}

	return (contentLength);
}

std::string RequestParser::extractBody(std::string & buffer, int contentLength)
{
	std::string body;

	if (buffer.empty()) {
		if (contentLength != 0)
			throw InvalidRequestException(400);
		body = "";
	}
	else {
		if (buffer.length() < contentLength)
			throw InvalidRequestException(400);
		body = buffer.substr(0, contentLength);
		buffer.erase(0, contentLength);
	}
	return (body);
}

// Function that returns a substring from the beginning to the first '\n' found
// (removing the '\r' before the '\n' if any)
// and truncates the input by removing the substring (and the '\n') from the it.
// Returns an empty string if the input is empty.
// Returns a copy of the whole input if there is no '\n', and earases the input.
std::string RequestParser::extractLineAndRemove(std::string & input)
{
	if (input.empty())
		return ("");

	size_t pos = input.find('\n');
	std::string	line;

	if (pos != std::string::npos) {
		line = input.substr(0, pos);
		if (!line.empty() && line.back() == '\r')
			line.resize(line.size() - 1);
		input.erase(0, pos + 1);
	}
	else {
		line = input;
		input.clear();
	}

	return (line);
}







/** For testing: g++ HttpRequest.cpp RequestParser.cpp httpStatusMessage.cpp **/

// int main()
// {
// 	std::string testCases[] = {
// 		"GET /index.html HTTP/1.1\r\n"
// 		"Host: localhost\r\n"
// 		"Connection: keep-alive\r\n"
// 		"\r\n",

// 		"POST /submit HTTP/1.1\r\n"
// 		"Host: localhost\r\n"
// 		"Content-Length: 11\r\n"
// 		"Content-Type: text/plain\r\n"
// 		"\r\n"
// 		"Hello World",

// 		"GET / HTTP/1.1\r\n"
// 		"Content-Length: 0\r\n"
// 		"\r\n",

// 		"POST / HTTP/1.1\r\n"
// 		"Host: localhost\r\n"
// 		"Content-Length: abc\r\n"
// 		"\r\n"
// 		"hello",

// 		"POST / HTTP/1.1\r\n"
// 		"Host: localhost\r\n"
// 		"Content-Length: 20\r\n"
// 		"\r\n"
// 		"short"
// 	};

// 	for (size_t i = 0; i < sizeof(testCases) / sizeof(testCases[0]); ++i) {
// 		std::cout << "\n==============================" << std::endl;
// 		std::cout << "Test Case #" << i + 1 << std::endl;
// 		std::cout << "==============================\n" << std::endl;

// 		try {
// 			RequestParser parser(testCases[i]);
// 			parser.getHttpRequest().debug(); // ✅ ta méthode HttpRequest::debug() appelée via wrapper
// 		}
// 		catch (const RequestParser::InvalidRequestException & e) {
// 			std::cerr << "❌ InvalidRequestException (" << e.getStatusCode() << ") : "
// 					  << e.what() << std::endl;
// 		}
// 	}
// 	return 0;
// }
