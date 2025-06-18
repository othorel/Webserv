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
# define MAX_HEADERS_SIZE 8192

enum ProcessStatus {
	WAITING_HEADERS,
	HANDLING_METHOD,
	WAITING_BODY,
	SENDING_HEADERS,
	SENDING_BODY,
	DONE
};

class ProcessRequest
{
	private :

		std::vector<ServerConfig>	_serversVector;
		ProcessStatus				_processStatus;
		ServerConfig				_server;
		int							_serverTimeout;
		Location					_location;
		std::string					_inputData;
		std::string					_outputData;
		HttpResponse				_httpResponse;
		typedef void	(ProcessRequest::*HandlerFunction)();
		HandlerFunction 			_handler;
		HttpRequest					*_request;
		File						*_file;

		void selectServer();
		void selectLocation();
		void selectHandler();
		const std::string & selectRoot();
		std::string selectErrorPage(int statusCode);

		void waitHeaders();
		void handleMethod();
		void waitBody();
		void sendHeaders();

		void sendBody();

		void deleteHandler();
		void getHandler();
		void postHandler();
		void cgiGetHandler();
		void cgiPostHandler();

		void buildResponse(int statusCode, const std::map<std::string, std::string> & headers, const std::string & body);
		void addFinalHeaders();
		void buildRedirect();

		void checkMethodValidity();
		std::string createPath();
		std::string createUploadPath();
		std::string createErrorFilePath(const std::string & errorPage);
		void checkPostValidity(const std::string & path);
		
	public :
		
		ProcessRequest();
		ProcessRequest(const std::vector<ServerConfig> & serversVector);
		ProcessRequest(const ProcessRequest & other);
		ProcessRequest & operator=(const ProcessRequest & other);
		~ProcessRequest();

		ProcessStatus getProcessStatus() const;
		const ServerConfig & getServer() const;
		int getServerTimeout() const;
		bool closeConection();
		std::string process(std::string data);
		void errorBuilder(int statusCode, bool secondTime = false);
		void reset();

};

#endif
