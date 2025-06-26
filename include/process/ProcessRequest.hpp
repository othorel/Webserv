#ifndef ProcessRequest_HPP
# define ProcessRequest_HPP

# include <exception>
# include <map>
# include <string>
# include "../http/HttpRequest.hpp"
# include "../http/HttpResponse.hpp"
# include "../http/ResponseBuilder.hpp"
# include "../config/Location.hpp"
# include "../config/ServerConfig.hpp"
# include "File.hpp"

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
		size_t selectMaxBodySize();

		void waitHeaders();
		void handleMethod();
		void waitBody();
		void writeBodyWithFile();
		void writeBodyWithoutFile();
		void sendHeaders();

		void sendBody();

		void deleteHandler();
		void getHandler();
		void postHandler();
		void handleUpload(const std::string & contentType);
		void cgiGetHandler();

		std::string createPath();
		std::string createUploadPath();
		std::string createUploadPathForClient();
		std::string createRelativeFilePath(const std::string & realPath);
		void checkPostValidity(const std::string & path);
		
	public :
		
		ProcessRequest();
		ProcessRequest(const std::vector<ServerConfig> & serversVector);
		ProcessRequest(const ProcessRequest & other);
		ProcessRequest & operator=(const ProcessRequest & other);
		~ProcessRequest();

		ProcessStatus getProcessStatus() const;
		const ServerConfig & getServer() const;
		File * getFilePtr() const;
		std::string process(std::string data);
		void errorBuilder(int statusCode, bool secondTime = false);

};

#endif
