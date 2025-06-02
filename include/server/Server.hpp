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

class ConfigParser;
class Server
{
	public:
		Server();
		Server(const ConfigParser & servconfig);
		Server(const Server & toCopy);

		~Server();
		
		Server & operator=(const Server & other);

		void	Setup();
		void	Run();
		void	dealClient(int fd, int & i);
		void	addNewConnexion(int fdClient);
		void	dealExistingClient(int fdClient, int & i);
		void	dealRequest(int fd);
		
	private:
		// const ConfigParser &						_serv;
		std::vector<int>							_fdSocketVect;
		std::vector<struct pollfd>					_fdPollVect;
		std::vector<std::pair<int, std::string>> 	_listenTab;

		void	addPair(std::pair<int, std::string>);

};



#endif