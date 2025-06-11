#ifndef ProcessRequest_HPP
# define ProcessRequest_HPP

# include <exception>
# include <map>
# include <string>
# include "../../include/http/HttpRequest.hpp"
# include "../../include/http/HttpResponse.hpp"
# include "../../include/config/Location.hpp"
# include "../../include/config/ServerConfig.hpp"
# include "../../include/http/File.hpp"

# define BUFFER_SIZE 4096

enum ProcessStatus {
	WAITING_HEADERS,
	REQUEST_READY,
	WAITING_BODY,
	RESPONSE_READY,
	SENDING_BODY,
	DONE
};

class ProcessRequest
{
	private :

		ProcessStatus	_processStatus;
		HttpRequest		*_request;
		ServerConfig	_server;
		Location		_location;
		typedef void	(ProcessRequest::*HandlerFunction)();
		HandlerFunction _handler;
		File			*_file;
		HttpResponse	_httpResponse;
		std::string		_rawString;

		void selectLocation();
		void selectHandler();
		const std::string & selectRoot();
		std::string selectErrorPage(int statusCode);

		void handle();
		void deleteHandler();
		void getHandler();
		void postHandler();

		size_t receiveBodyChunk(char * buffer, size_t writesize);
		const std::string & sendHttpResponse();
		size_t sendBodyChunk(char * buffer, size_t readsize);

		void buildResponse(int statusCode, const std::map<std::string, std::string> & headers, const std::string & body);
		void buildRedirect();
		// void buildError(int statusCode, const ServerConfig & server, const Location * location);
		
	public :
		
		ProcessRequest();
		ProcessRequest(const ServerConfig & server);
		ProcessRequest(const ProcessRequest & other);
		ProcessRequest & operator=(const ProcessRequest & other);
		// void reset(const HttpRequest & request, const std::vector<ServerConfig> & serverVector);
		// ~ProcessRequest();

		ProcessStatus getProcessStatus() const;
		std::string process(std::string data);


};

static std::string createAllowedMethodsList(const Location & location);
static void checkDeleteValidity(const std::string & path);
static std::string createIndexPath(std::string path, const Location & location);
static std::string generateAutoIndex(const std::string & dirPath, const std::string & uriPath);
static std::string sanitizeFilenamePart(const std::string & input);
static void checkPostValidity(
	const HttpRequest & request, const Location & location ,
	const ServerConfig & server, const std::string & path);
static std::string createPostFileName(
	const HttpRequest & request, const ServerConfig & server, const std::string & path);
static std::string createPath(const std::string & root, const std::string & subpath);

#endif
