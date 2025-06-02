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
		class	ServerSetupException : public std::exception
		{
			public:
				const char * what() const throw()
				{
					return ("Error while creating socket");
				}
		};
		

		
	private:
		// const ConfigParser &						_serv;
		std::vector<int>							_fdSocketVect;
		std::vector<std::pair<int, std::string>> 	_listenTab;

		void	addPair(std::pair<int, std::string>);

};



#endif