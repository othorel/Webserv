#ifndef CGIHANDLER_HPP
# define CGIHANDLER_HPP

# include <string>
# include <vector>
# include <map>
# include <exception>
# include <unistd.h>
# include <sys/wait.h>
# include <cstdlib>
# include <cstdio>
# include <iostream>
# include <ctime>
# include <sys/time.h>
# include <sys/select.h>

# include "../http/HttpRequest.hpp"
# include "../http/HttpResponse.hpp"

class CGIHandler
{

	private:

		HttpRequest		_request;
		std::string		_scriptPath;
		std::string		_queryString;
		HttpResponse	_response;
		
		void buildResponse();
		std::string execute();
		std::vector<std::string> buildEnv();

	public:

		CGIHandler();
		CGIHandler(const HttpRequest & request, const std::string & path);
		CGIHandler(const CGIHandler & other);
		CGIHandler & operator=(const CGIHandler & other);

		const HttpResponse & getHttpResponse();
};

#endif
