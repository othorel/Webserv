#include <iostream>
#include <sstream>
#include <string>
#include <map>
#include <exception>


// Function that returns an HTTP error message according to an HTTP error code
std::string getStatusMessage(int code)
{
	switch (code)
	{
		// --- 1xx: Informational ---
		case 100: return "Continue";
		case 101: return "Switching Protocols";
		case 102: return "Processing";                      // RFC 2518
		case 103: return "Early Hints";                     // RFC 8297

		// --- 2xx: Success ---
		case 200: return "OK";
		case 201: return "Created";
		case 202: return "Accepted";
		case 203: return "Non-Authoritative Information";
		case 204: return "No Content";
		case 205: return "Reset Content";
		case 206: return "Partial Content";
		case 207: return "Multi-Status";                    // RFC 4918
		case 208: return "Already Reported";                // RFC 5842
		case 226: return "IM Used";                         // RFC 3229

		// --- 3xx: Redirection ---
		case 300: return "Multiple Choices";
		case 301: return "Moved Permanently";
		case 302: return "Found";
		case 303: return "See Other";
		case 304: return "Not Modified";
		case 305: return "Use Proxy";
		case 307: return "Temporary Redirect";
		case 308: return "Permanent Redirect";              // RFC 7538

		// --- 4xx: Client Error ---
		case 400: return "Bad Request";
		case 401: return "Unauthorized";
		case 402: return "Payment Required";
		case 403: return "Forbidden";
		case 404: return "Not Found";
		case 405: return "Method Not Allowed";
		case 406: return "Not Acceptable";
		case 407: return "Proxy Authentication Required";
		case 408: return "Request Timeout";
		case 409: return "Conflict";
		case 410: return "Gone";
		case 411: return "Length Required";
		case 412: return "Precondition Failed";
		case 413: return "Payload Too Large";
		case 414: return "URI Too Long";
		case 415: return "Unsupported Media Type";
		case 416: return "Range Not Satisfiable";
		case 417: return "Expectation Failed";
		case 418: return "I'm a teapot";                    // RFC 7168
		case 421: return "Misdirected Request";             // RFC 7540
		case 422: return "Unprocessable Entity";            // RFC 4918
		case 423: return "Locked";                          // RFC 4918
		case 424: return "Failed Dependency";               // RFC 4918
		case 425: return "Too Early";                       // RFC 8470
		case 426: return "Upgrade Required";
		case 428: return "Precondition Required";           // RFC 6585
		case 429: return "Too Many Requests";               // RFC 6585
		case 431: return "Request Header Fields Too Large"; // RFC 6585
		case 451: return "Unavailable For Legal Reasons";   // RFC 7725

		// --- 5xx: Server Error ---
		case 500: return "Internal Server Error";
		case 501: return "Not Implemented";
		case 502: return "Bad Gateway";
		case 503: return "Service Unavailable";
		case 504: return "Gateway Timeout";
		case 505: return "HTTP Version Not Supported";
		case 506: return "Variant Also Negotiates";         // RFC 2295
		case 507: return "Insufficient Storage";            // RFC 4918
		case 508: return "Loop Detected";                   // RFC 5842
		case 510: return "Not Extended";                    // RFC 2774
		case 511: return "Network Authentication Required"; // RFC 6585

		default: return "Unknown Status";
	}
}












class HttpRequest
{
	private :

		std::string							_method;
		std::string							_uri;
		std::string							_version;
		std::map<std::string, std::string>	_headers;
		std::string							_body;
		unsigned int						_contentLength;
		
		HttpRequest();
		void		parseRequestLine(std::string & buffer);
		std::string	extractLineAndRemove(std::string & input);
		void		parseHeaders(std::string & buffer);
		void		affectContentLength();
		void		parseBody(std::string & buffer);

	public :

		~HttpRequest();
		HttpRequest(const HttpRequest & other);
		HttpRequest & operator=(const HttpRequest & other);

		HttpRequest(const std::string & raw_request);

		std::string	getMethod() const;
		std::string	getUri() const;
		std::string	getVersion() const;
		std::string	getBody() const;
		std::string	getHeaderValue(const std::string & key) const;
		bool		hasHeader(const std::string & key) const;
		size_t		getBodyLength() const;
		void		debug() const;

		class InvalidRequestException : std::exception
		{
			private :

				int			_statusCode;
				std::string	_msg;

			public :

				InvalidRequestException(int code) :
					_statusCode(code),
					_msg(getStatusMessage(code))
				{}

				virtual const char * what() const throw()
				{
					return (_msg.c_str());
				}

				int	getStatusCode() const
				{
					return (_statusCode);
				}
		};
};

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

// Method that creates the object by parsing the whole request
// throwing invalidRequestException in case of error
HttpRequest::HttpRequest(const std::string & raw_request)
{
	if (raw_request.empty())
		throw InvalidRequestException(400);
	
	std::string buffer = raw_request;
	parseRequestLine(buffer);
	parseHeaders(buffer);
	affectContentLength();
	parseBody(buffer);
}

// Function that returns a substring from the beginning to the first '\n' found
// (removing the '\r' before the '\n' if any)
// and truncates the input by removing the substring (and the '\n') from the it.
// Returns an empty string if the input is empty.
// Returns a copy of the whole input if there is no '\n', and earases the input.
std::string HttpRequest::extractLineAndRemove(std::string & input)
{
	if (input.empty())
		return ("");

	size_t pos = input.find('\n');
	std::string	line;

	if (pos != std::string::npos) {
		line = input.substr(0, pos);
		if (line.back() == '\r')
			line.resize(line.size() - 1);
		input.erase(0, pos + 1);
	}
	else {
		line = input;
		input.clear();
	}

	return (line);
}

// Method that parses the request line of the request
// throwing invalidRequestException in case of error
void HttpRequest::parseRequestLine(std::string & buffer)
{
	std::string requestLine = extractLineAndRemove(buffer);
	if (buffer.empty())
		throw InvalidRequestException(400);
	std::istringstream iss(requestLine);
	if (!(iss >> _method))
		throw InvalidRequestException(400);
	if (!(iss >> _uri))
		throw InvalidRequestException(400);
	if (!(iss >> _version))
		throw InvalidRequestException(400);
	if (_version != "HTTP/1.1")
		throw InvalidRequestException(400);
	std::string shouldBeEmpty;
	if ((iss >> shouldBeEmpty))
		throw InvalidRequestException(400);
}

// Method that parses the header lines of the request
// throwing invalidRequestException in case of error
void HttpRequest::parseHeaders(std::string & buffer)
{
	if (buffer.empty())
		throw InvalidRequestException(400);

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
		if (pos != std::string::npos)
			value = value.substr(pos);
		else
			value = "";
		
		_headers[key] = value;
	}
	if (!hasHeader("host") || getHeaderValue("host") == "")
		throw InvalidRequestException(400);
}

// Method that affects the _contentLength value of the HttpRequest object
// based on the header "content-length" of the request
// (or 0 if there is no such header)
// throwing invalidRequestException in case of error
void HttpRequest::affectContentLength()
{
	if (hasHeader("content-length")) {
		std::string contentLengthValue = getHeaderValue("content-length");
		if (contentLengthValue.empty())
			throw InvalidRequestException(400);
		for (size_t i = 0; i < contentLengthValue.length(); i++) {
			if (!std::isdigit(contentLengthValue[i]))
				throw InvalidRequestException(400);
		}
		std::istringstream iss(contentLengthValue);
		iss >> _contentLength;
	}
	else
		_contentLength = 0;
}

// Method that parses the body ot the request
// throwing invalidRequestException in case of error
void HttpRequest::parseBody(std::string & buffer)
{
	if (buffer.empty()) {
		if (_contentLength != 0)
			throw InvalidRequestException(400);
	}
	else {
		if (buffer.length() < _contentLength)
			throw InvalidRequestException(400);
		_body = buffer.substr(0, _contentLength);
		buffer.erase(0, _contentLength);
	}
}

std::string HttpRequest::getMethod() const
{
	return (_method);
}

std::string HttpRequest::getUri() const
{
	return (_uri);
}

std::string HttpRequest::getVersion() const
{
	return (_version);
}

std::string HttpRequest::getBody() const
{
	return (_body);
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
	return (_body.length());
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









int main(int ac, char **av)
{
	if (ac != 2) {
		std::cerr << "1 argument needed (a whole HTTP request string)" << std::endl;
		return 1;
	}
	std::string request(av[1]);
	try {
		HttpRequest httpRequest(request);
		httpRequest.debug();
	}
	catch (const HttpRequest::InvalidRequestException & e) {
		std::cerr << "Invalid HTTP request (" 
		          << e.getStatusCode() << "): " 
		          << e.what() << std::endl;
		return e.getStatusCode();
	}
	return 0;
}
