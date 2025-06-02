#ifndef SERVER_HPP
# define SERVER_HPP

# include <iostream>
# include <sys/types.h>
# include <sys/socket.h>
# include <netinet/in.h>
# include <string>

class ServerConfig;
class Server
{
	public:
		Server();
		Server(const ServerConfig & servconfig);
		Server(const Server & toCopy);

		~Server();
		
		Server & operator=(const Server & other);
		
	private:

	
		// const std::string &		_serverName;
		// const std::string &		_host;
		// const std::string &		_root;
		// int						_port;
		// struct sockaddr_in		_servAddr;
		// int						_socketFd;

};

#endif