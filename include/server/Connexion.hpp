
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
		Connexion(int fd, sockaddr_in addr, const std::vector<ServerConfig> & vectServerConfig);
		~Connexion();
		Connexion(const Connexion & toCopy);
		Connexion & operator=(const Connexion & other);

		// Getters
		int								getFd() const;
		std::string						getIP() const;
		int								getPort() const;
		sockaddr_in						getAddr() const;
		ProcessRequest					*getProcessRequest() const;
		ssize_t							getBytesIn() const;
		ssize_t							getBytesOut() const;
		std::string						getBufferIn() const;
		std::string						getBufferOut() const;
		ServerConfig					*getServConfig() const;
		std::vector<ServerConfig>		getServConfigVect() const;
		std::time_t						getStartTime() const;

		// Setters
		void							setBytesIn(ssize_t bytes);
		void							setBytesOut(ssize_t bytes);
		void							setBufferIn(std::string buffer);
		void							setBufferOut(std::string buffer);
		void							setServConfig(ServerConfig *serverconfig);
		void							setProcessRequest();

		// Runtime
		void							readDataFromSocket(std::string &line);
		void							writeDataToSocket(const std::string & response);
		bool							endTransmission(std::string line);

	private:

		int								_fd;
		sockaddr_in						_addr;
		std::time_t						_startTime;
		std::vector<ServerConfig>		_serverConfigVect;
		ServerConfig					*_servConfig;
		ProcessRequest					*_processRequest;

		ssize_t							_bytesIn;
		ssize_t							_bytesOut;
		std::string						_bufferIn;
		std::string						_bufferOut;

};

#endif