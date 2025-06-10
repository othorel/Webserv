
#ifndef CONNEXION_HPP
# define CONNEXION_HPP

# include <iostream>
# include <sys/types.h>
# include <sys/socket.h>
# include <netinet/in.h>
# include <string>
# include <vector>
# include <map>
# include <algorithm>
# include <exception>
# include <arpa/inet.h>
# include <poll.h>
# include <unistd.h>
# include <ctime>

# include "../config/ServerConfig.hpp"

class RequestParser;
class HttpRequest;
class ProcessRequest;
class Connexion
{
	public:
		Connexion();
		// Connexion(int fd, sockaddr_in addr, const ServerConfig &servconfig);
		Connexion(int fd, sockaddr_in addr);
		~Connexion();
		Connexion(const Connexion & toCopy);
		Connexion & operator=(const Connexion & other);

		// Getters
		int					getFd() const;
		std::string			getIP() const;
		int					getPort() const;
		std::string			getBufferIn() const;
		// std::string			getBufferOut() const;
		const sockaddr_in	&getAddr() const;
		bool				isHeaderParsed() const;

		std::string			getHeaders() const;
		std::string			getBody() const;

		ServerConfig		getServConfig() const;
		HttpRequest			*getHttpRequest() const;
		Location			*getLocation() const;
		ProcessRequest		*getProcessRequest() const;
		std::time_t			getStartTime() const;

		// Setters
		void				setRequestParser(HttpRequest *request);
		void				setHeaderParsed();
		void				appendRaw(std::string attribute, std::string content);
		void				setProcessRequest(ProcessRequest *processrequest);

		// void			setBody();

		// Runtime
		ssize_t				readDataFromSocket(std::string &line);
		ssize_t				writeDataToSocket(const std::string & response);
		bool				endTransmission();

	private:
		bool				_endTransmission;
		bool				_headerIsParsed;
		int					_fd;
		sockaddr_in			_addr;
		std::string			_bufferIn;
		std::string			_bufferOut;
		std::string			_headers;
		std::string			_body;

		HttpRequest			*_request;
		Location			*_location;
		ProcessRequest		*_processRequest;
		// ServerConfig		*_servConfig;
		
		std::time_t			_startTime;
		// ServerConfig		_servConfig;
};

#endif