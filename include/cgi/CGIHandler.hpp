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

# include "../http/HttpRequest.hpp"

class CGIHandler {

	private:

		std::string _scriptPath;
		std::string _method;
		std::string	_target;
		std::string _version;
		std::string _body;
		std::string	_queryString;
		std::map<std::string, std::string> _headers;

		std::vector<std::string> buildEnv() const;

	public:

		CGIHandler();
		CGIHandler(const std::string & path, const HttpRequest & request);

		std::string execute();
	
	class CGIException : public std::exception {

		private:

			std::string _msg;

		public:

			CGIException(const std::string& msg) : _msg(msg) {}
			virtual ~CGIException() throw() {}
			virtual const char* what() const throw() {return _msg.c_str(); }
	};
		
};

#endif