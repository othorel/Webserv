#ifndef RESPONSEBUILDER_HPP
# define RESPONSEBUILDER_HPP

# include "../process/ProcessRequest.hpp"

class ProcessRequest;

class ResponseBuilder
{
	public :

		static void buildResponse(ProcessRequest * process, int statusCode,
			const std::map<std::string, std::string> & headers, const std::string & body);
		static void buildRedirect(ProcessRequest * process);
		static void errorBuilder(ProcessRequest * process, int statusCode, bool secondTime);
		static void addFinalHeaders(ProcessRequest * process);

	private :

		static std::string createErrorFilePath(ProcessRequest * process, const std::string & errorPage);
		static std::string selectErrorPage(ProcessRequest * process, int statusCode);
		static const std::string & selectRoot(ProcessRequest * process);

		ResponseBuilder();
		ResponseBuilder(const ResponseBuilder &);
		ResponseBuilder & operator=(const ResponseBuilder &);

};

#endif
