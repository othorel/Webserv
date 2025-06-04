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
		
		RequestParser();
		void parseRequest(const std::string & raw_request);
		std::string	extractLineAndRemove(std::string & input);
		std::string extractMethod(std::istringstream & iss);
		std::string extractUri(std::istringstream & iss);
		std::string extractVersion(std::istringstream & iss);
		std::map<std::string, std::string> extractHeaders(std::string & buffer);
		int calculateContentLength(const std::map<std::string, std::string> & headers);
		std::string extractBody(std::string & buffer, int contentLength);

	public :
	
		RequestParser(const std::string & raw_request);
		RequestParser(const RequestParser & other);
		RequestParser & operator=(const RequestParser & other);
		~RequestParser();

		const HttpRequest & getHttpRequest() const;

		class InvalidRequestException : std::exception
		{
			private :
				int			_statusCode;
				std::string	_msg;
			public :
				InvalidRequestException(int code) :
					_statusCode(code),
					_msg(httpStatusMessage(code))
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

#endif
