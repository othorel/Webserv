#ifndef SERVER_HPP
# define SERVER_HPP

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

class ConfigParser;
class ServerConfig;
class Connexion;
class PollManager;

class Server
{
	public:
		Server();
		Server(const ConfigParser & Parser, const std::vector<ServerConfig> servConfigVect);
		Server(const Server & toCopy);
		Server(const std::string str);

		~Server();
		
		Server & operator=(const Server & other);

		void	StartEventLoop();

		std::vector<std::pair<int, std::string> >	getListenVect() const;
		std::vector<std::pair<int, std::string> >	getActiveListenVec() const;
		PollManager									*getPollManager() const;
		std::map<int, Connexion>					getClientsMap() const;
		std::vector<int>							getFdSocketVect() const;
		std::vector<ServerConfig> 					getServerConfig() const;

		void										setServerConfig(std::vector<ServerConfig> & servConfigVect);
		
	private:
		// Attributes
		std::vector<std::pair<int, std::string> > 	_listenVect; // couples port-IP a surveiller
		std::vector<int>							_fdSocketVect; // si tout se passe bien doit faire la meme taille que _listenVect (sauf si certains coupls port ip ne ne sont pas accessibles)
		
		
		PollManager									*_pollManager; // classe contenant un vect de pollFd
		std::map<int, Connexion>					_clientsMap;

		std::vector<std::pair<int, std::string> >	_activeListenVect; // sous-partie de listen tab contenant uniquement les couples ports IP concernes par des evenements
		std::vector<ServerConfig>					_serverConfigVect;		

		// Initialization
		void	Setup();
		void	addPair(std::pair<int, std::string> listen);
		void	fillActiveListenVect();

		// Runtime
		void	dealClient(int fd, size_t & i);
		void	acceptNewConnexion(int fd);
		void	handleEvent(int fdClient, size_t & i);
		void	dealRequest(int fd);	

};

#endif