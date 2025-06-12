#ifndef REQUESTPARSER_HPP
# define REQUESTPARSER_HPP

# include <string>
# include <map>
# include <exception>
# include "HttpRequest.hpp"

class RequestParser
{
	private :

		HttpRequest	* _httpRequest;
		
		void parseRequest(const std::string & raw_request);
		RequestParser(const RequestParser & other);
		RequestParser & operator=(const RequestParser & other);
		
	public :
		
		RequestParser();
		RequestParser(const std::string & raw_request);
		~RequestParser();

		const HttpRequest & getHttpRequest() const;
		HttpRequest *release();

};

#endif
