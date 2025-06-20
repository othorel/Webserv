#ifndef ProcessRequest_HPP
# define ProcessRequest_HPP

# include <exception>
# include <map>
# include <string>
# include "../../include/http/HttpRequest.hpp"
# include "../../include/http/HttpResponse.hpp"
# include "../../include/http/ResponseBuilder.hpp"
# include "../../include/config/Location.hpp"
# include "../../include/config/ServerConfig.hpp"
# include "../../include/http/File.hpp"

# define BUFFER_SIZE 4096
# define MAX_HEADERS_SIZE 8192

class ResponseBuilder;

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

	friend class ResponseBuilder;

	private :

		std::vector<ServerConfig>	_serversVector;
		ProcessStatus				_processStatus;
		ServerConfig				_server;
		int							_serverTimeout;
		Location					_location;
		std::string					_inputData;
		std::string					_outputData;
		HttpResponse				_httpResponse;
		size_t						_bytesSent;
		typedef void	(ProcessRequest::*HandlerFunction)();
		HandlerFunction 			_handler;
		HttpRequest					*_request;
		File						*_file;

		void selectServer();
		void selectLocation();
		void selectHandler();
		const std::string & selectRoot();
		std::string selectErrorPage(int statusCode);
		size_t selectMaxBodySize();

		void waitHeaders();
		void handleMethod();
		void waitBody();
		void sendHeaders();

		void sendBody();

		void deleteHandler();
		void getHandler();
		void postHandler();
		void cgiGetHandler();

		void checkMethodValidity();
		std::string createPath();
		std::string createUploadPath();
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
		File * getFilePtr() const;
		bool closeConection();
		std::string process(std::string data);
		void errorBuilder(int statusCode, bool secondTime = false);
		void reset();

};

#endif
