#ifndef SERVER_HPP
# define SERVER_HPP

# include <iostream>
# include <sys/types.h>
# include <sys/socket.h>
# include <netinet/in.h>
# include <string>
# include <vector>
# include <algorithm>
# include <exception>
# include <arpa/inet.h>
# include <poll.h>
# include <unistd.h>

// class ConfigParser;
class Connexion;
class PollManager;

class Server
{
	public:
		Server();
		// Server(const ConfigParser & servconfig);
		Server(const Server & toCopy);
		Server(const std::string str);

		~Server();
		
		Server & operator=(const Server & other);

		void	StartEventLoop();
		
	private:
		// Attributes
		PollManager									_pollManager;
		std::vector<int>							_fdSocketVect;
		// std::vector<struct pollfd>				_fdPollVect;
		std::vector<std::pair<int, std::string> > 	_listenTab;

		// Initialization
		void	Setup();
		void	addPair(std::pair<int, std::string> listen);

		// Runtime
		void	dealClient(int fd, int & i);
		void	acceptNewConnexion(int fd);
		void	handleEvent(int fdClient, int & i);
		void	dealRequest(int fd);	

};

#endif