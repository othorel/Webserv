#ifndef REQUESTPARSER_HPP
# define REQUESTPARSER_HPP

# include <string>
# include <map>
# include <exception>
# include "HttpRequest.hpp"

std::string httpStatusMessage(int code);

class RequestParser
{
	private :

		HttpRequest	_httpRequest;
		
		void parseRequest(const std::string & raw_request);
		
		public :
		
		RequestParser();
		RequestParser(const std::string & raw_request);
		RequestParser(const RequestParser & other);
		RequestParser & operator=(const RequestParser & other);
		~RequestParser();

		const HttpRequest & getHttpRequest() const;

};

static std::string	extractLineAndRemove(std::string & input);
static std::string extractMethod(std::istringstream & iss);
static std::string extractUri(std::istringstream & iss);
static std::string extractVersion(std::istringstream & iss);
static std::map<std::string, std::string> extractHeaders(std::string & buffer);
static int calculateContentLength(const std::map<std::string, std::string> & headers);
static std::string extractBody(std::string & buffer, int contentLength);

#endif
