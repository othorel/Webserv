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
# include <ctime>


# ifdef __APPLE__
	extern volatile sig_atomic_t g_stop;
# else
	extern volatile __sig_atomic_t g_stop;
# endif

class ConfigParser;
class ServerConfig;
class Connexion;
class PollManager;

class Server
{
	public:
		Server();
		Server(const ConfigParser & Parser);
		Server(const Server & toCopy);
		~Server();
		Server & operator=(const Server & other);

		// Runtime
		void										StartEventLoop();

		// Logs
		void										logTime() const;
		void										announce() const;

		// Getters
		std::vector<std::pair<int, std::string> >	getListenVect() const;
		PollManager									*getPollManager() const;
		std::map<int, Connexion>					getClientsMap() const;
		std::vector<int>							getFdSocketVect() const;
		const std::vector<ServerConfig> 			*getServerConfig() const;
		
		// Setters
		void										setServerConfig(const std::vector<ServerConfig> *servConfigVect);
		std::vector<ServerConfig>					ActiveServConfigVect(int port, std::string IP);
		
	private:
		// Attributes
		std::vector<std::pair<int, std::string> > 	_listenVect; // couples port-IP a surveiller
		std::vector<int>							_fdSocketVect; // si tout se passe bien doit faire la meme taille que _listenVect (sauf si certains coupls port ip ne ne sont pas accessibles)
		PollManager									*_pollManager; // classe permettant de manipuler mes sockets
		std::map<int, Connexion>					_clientsMap; // Tous les clients actuellement en train de communiquer
		const std::vector<ServerConfig>				*_serverConfigVect;

		// Initialization
		void										Setup();
		void										addPair(std::pair<int, std::string> listen);

		// Runtime
		void										dealClient(int fd, size_t & i, short revents);
		void										acceptNewConnexion(int fd);
		void										handleEventPOLLIN(int fdClient, size_t & i);
		void										handleEventPOLLOUT(int fdClient, size_t & i);
		void										handleError(int errorCode, int fdClient, size_t & i);
		void										supressClient(int fdClient, size_t & i);
		void										readSocket(int fd, std::string & rawLine, size_t & i);

		//Logs
		void										readLog(int fdClient);

};

#endif