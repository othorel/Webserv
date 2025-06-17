
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
# include "../http/ProcessRequest.hpp"

class RequestParser;

class Connexion
{
	public:
		Connexion();
		Connexion(int fd, sockaddr_in addr, const std::vector<ServerConfig> *_serverConfigVect);
		Connexion(const Connexion & toCopy);
		Connexion &operator=(const Connexion & other);
		~Connexion();

		// Getters
		int								getFd() const;
		std::string						getIP() const;
		int								getClientPort() const;
		int								getLocalPort() const;
		sockaddr_in						getAddr() const;
		ProcessRequest					&getProcessRequest();
		const ProcessRequest			&getProcessRequest() const;
		ssize_t							getBytesIn() const;
		ssize_t							getBytesOut() const;
		std::string						getBufferIn() const;
		std::string						getBufferOut() const;
		const ServerConfig				*getServConfig() const;
		std::time_t						getStartTime() const;
		
		int								getEndPreviousRequest() const;
		int								getNbRequests() const;

		// Setters
		void							setBytesIn(ssize_t bytes);
		void							setBytesOut(ssize_t bytes);
		void							setBufferIn(std::string buffer);
		void							setBufferOut(std::string buffer);
		void							setServConfig(const ServerConfig *serverconfig);
		void							increaseNbRequests();
		void							actualizeEndPreviousRequest();

		// Runtime
		void							readDataFromSocket(std::string &line);
		void							writeDataToSocket(const std::string & response);
		bool							endTransmission(std::string line);
		
		
		int								keepAliveTimeOut;
		int								keepAliveMaxRequests;

	private:

		int								_fd;
		sockaddr_in						_addr;
		std::time_t						_startTime;
		const ServerConfig				*_servConfig;
		ProcessRequest					_processRequest;

		ssize_t							_bytesIn;
		ssize_t							_bytesOut;
		std::string						_bufferIn;
		std::string						_bufferOut;

		int								_endPreviousRequest;
		int								_nbRequests;

		

		
		
};

#endif