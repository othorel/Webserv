
#include "../../include/server/Server.hpp"
#include "../../include/server/PollManager.hpp"
#include "../../include/server/Connexion.hpp"
#include "../../include/config/ServerConfig.hpp"
#include "../../include/config/ConfigParser.hpp"
#include "../../include/http/RequestParser.hpp"
#include "../../include/process/ProcessRequest.hpp"
#include "../../include/http/HttpErrorException.hpp"

static void	strToLower(char *str);
static int getLocalPort(int fd);

////////////////////////////////////////////////////////////////////////////////
///                               CANONIC +                                  ///
////////////////////////////////////////////////////////////////////////////////

Server::Server()
{
	_pollManager = NULL;
	_serverConfigVect = NULL;
}

Server::Server(const ConfigParser & Parser) :
	_pollManager(new PollManager()),
	_serverConfigVect(&Parser.getServerConfigVector())
{
	std::vector<ServerConfig>::const_iterator	it = (*_serverConfigVect).begin();
	while (it != (*_serverConfigVect).end())
	{
		addPair(it->getListen());
		it++;
	}
	Setup();
}

Server::Server(const Server & toCopy)
{
	*this = toCopy;
}

Server::~Server()
{
	if (_pollManager)
		delete _pollManager;
}
		
Server & Server::operator=(const Server & other)
{
	if (this != &other)
	{
		this->_pollManager = NULL;
		this->_fdSocketVect = other._fdSocketVect;
		this->_listenVect = other._listenVect;
		this->_clientsMap = other._clientsMap;
		this->_serverConfigVect = NULL;
	}
	return (*this);
}

////////////////////////////////////////////////////////////////////////////////
///                                 GETTERS                                  ///
////////////////////////////////////////////////////////////////////////////////

std::vector<std::pair<int, std::string> >	Server::getListenVect() const
{
	return (this->_listenVect);
}

PollManager									*Server::getPollManager() const
{
	return (this->_pollManager);
}

std::map<int, Connexion>					Server::getClientsMap() const
{
	return (this->_clientsMap);
}

std::vector<int>							Server::getFdSocketVect() const
{
	return (this->_fdSocketVect);
}

const std::vector<ServerConfig> 			*Server::getServerConfig() const
{
	return (this->_serverConfigVect);
}

////////////////////////////////////////////////////////////////////////////////
///                                 SETTERS                                  ///
////////////////////////////////////////////////////////////////////////////////

void	Server::setServerConfig(const std::vector<ServerConfig> *servConfigVect)
{
	this->_serverConfigVect = servConfigVect;
}

////////////////////////////////////////////////////////////////////////////////
///                                RUNTIME                                   ///
////////////////////////////////////////////////////////////////////////////////

void	Server::StartEventLoop()
{
	while (!g_stop)
	{
		_pollManager->pollExec(50);
		for (size_t i = 0; i != _pollManager->getPollFdVector().size(); i++)
		{
			int		fd = _pollManager->getPollFdVector()[i].fd;
			short	revents = _pollManager->getPollFdVector()[i].revents;
			if (revents & POLLIN && fd == STDIN_FILENO)
			{
					char	buffer[256];
					if (fgets(buffer, sizeof(buffer), stdin))
					{
						buffer[strcspn(buffer, "\n")] = 0;
						strToLower(buffer);
						if (strcmp(buffer, "exit") == 0)
							return;
					}
			}
			else if (revents & (POLLIN | POLLOUT))
			{
				dealClient(fd, i, revents);
			}
		}
	}
}

void	Server::dealClient(int fd, size_t & i, short revents)
{
	if (std::find(_fdSocketVect.begin(), _fdSocketVect.end(), fd) != _fdSocketVect.end())
	{
		logTime();
		std::cout << "[INFO]\t\tNew connexion request" << std::endl;
		acceptNewConnexion(fd);
	}
	else if (revents & POLLIN)
		handleEventPOLLIN(fd, i);
	else if (revents & POLLOUT)
		handleEventPOLLOUT(fd, i);
	else
		throw std::runtime_error("Revents error");
}

void	Server::acceptNewConnexion(int fd)
{
	struct sockaddr_in clientAddr;

	socklen_t addrLen = sizeof(clientAddr);
	int clientFd = accept(fd, (struct sockaddr*)&clientAddr, &addrLen);
	if (clientFd < 0)
		return;
	_pollManager->addSocket(clientFd, POLLIN);
	std::vector<ServerConfig> ActiveVect = ActiveServConfigVect(getLocalPort(fd), inet_ntoa(clientAddr.sin_addr));
	_clientsMap[clientFd] = Connexion(clientFd, clientAddr, ActiveVect);
	std::cout << "\033[32m";
	logTime();
	
	std::cout << "[INFO]\t\tNew client "
			  << _clientsMap[clientFd].getClientPort()
			  << " authorized on: "
			  << _clientsMap[clientFd].getIP() << ":"
			  << _clientsMap[clientFd].getLocalPort()
			  << "\033[0m" << std::endl;
}

void	Server::handleEventPOLLIN(int fdClient, size_t & i)
{
	std::string		rawLine;

	try
	{
		readSocket(fdClient, rawLine, i);

		_clientsMap[fdClient]._processed = _clientsMap[fdClient].getProcessRequest().process(rawLine);

		// request log
		if (!_clientsMap[fdClient]._processed.empty())
		{
			std::string requestLine = "";
			size_t pos = _clientsMap[fdClient]._processed.find("\r\n");
			if (pos != std::string::npos)
				requestLine = _clientsMap[fdClient]._processed.substr(0, pos);
			logTime();
			std::cout <<  "[REQUEST]\t\t" << requestLine << std::endl;
			_pollManager->setState(fdClient, POLLOUT);
			return;
		}
	}
	catch (const HttpErrorException& e)
	{
		logTime();
		std::cout << "[EXCEPTION]\t" << e.getStatusCode() << ": "<< e.what() << std::endl;

		handleError(e.getStatusCode(), fdClient, i);
		return;
	}
}

void	Server::handleEventPOLLOUT(int fdClient, size_t & i)
{
	int				status = 0;
	std::string		rawLine;
	try
	{
		if (_clientsMap[fdClient].getServConfig() == NULL)
			_clientsMap[fdClient].setServConfig(&_clientsMap[fdClient].getProcessRequest().getServer());

		//CATCH AND WRITE RESPONSE
		while (!_clientsMap[fdClient]._processed.empty())
		{
			_clientsMap[fdClient].writeDataToSocket(_clientsMap[fdClient]._processed);
			_clientsMap[fdClient]._processed = _clientsMap[fdClient].getProcessRequest().process(rawLine);
			status = _clientsMap[fdClient].getProcessRequest().getProcessStatus();
		}
		
		//CATCH AND WRITE RESPONSE
		
		if (status == 5)
			supressClient(fdClient, i);			
	}
	catch (const HttpErrorException& e)
	{
		logTime();
		std::cout << "[EXCEPTION]\t" << e.getStatusCode() << ": "<< e.what() << std::endl;

		handleError(e.getStatusCode(), fdClient, i);
		return;
	}
}

void	Server::handleError(int errorCode, int fdClient, size_t & i)
{
	if (_clientsMap[fdClient].getProcessRequest().getFilePtr())
		_clientsMap[fdClient].getProcessRequest().getFilePtr()->closeFile();
	try
	{
		
		_clientsMap[fdClient].getProcessRequest().errorBuilder(errorCode);
		std::string	processed = _clientsMap[fdClient].getProcessRequest().process("");
		int status = _clientsMap[fdClient].getProcessRequest().getProcessStatus();
		
		if (!processed.empty())
		{
			while (!processed.empty())
			{
				_clientsMap[fdClient].writeDataToSocket(processed);
				processed = _clientsMap[fdClient].getProcessRequest().process("");
				status = _clientsMap[fdClient].getProcessRequest().getProcessStatus();
				(void)status;
			}
		}
		else
		{
			;
		}
	}
	catch(const HttpErrorException& e)
	{
		_clientsMap[fdClient].getProcessRequest().errorBuilder(errorCode, true);

		std::string	processed = _clientsMap[fdClient].getProcessRequest().process("");
		int status = _clientsMap[fdClient].getProcessRequest().getProcessStatus();
				(void)status;

		
		while (!processed.empty())
		{
			_clientsMap[fdClient].writeDataToSocket(processed);
			processed = _clientsMap[fdClient].getProcessRequest().process("");
			status = _clientsMap[fdClient].getProcessRequest().getProcessStatus();
		}
	}
	supressClient(fdClient, i);
}

void	Server::readSocket(int fd, std::string & rawLine, size_t & i)
{
	_clientsMap[fd].readDataFromSocket(rawLine); 

	if (_clientsMap[fd].getBytesIn() <= 0)
	{
		supressClient(fd, i);
		if (_clientsMap[fd].getBytesIn() < 0)
			throw std::runtime_error("Error while reading from socket");
		return ;
	}
}

std::vector<ServerConfig> Server::ActiveServConfigVect(int Port, std::string IP)
{
	std::vector<ServerConfig>	vect;

	for (std::vector<ServerConfig>::const_iterator it = _serverConfigVect->begin(); it != _serverConfigVect->end(); ++it)
	{
		const std::pair<int, std::string> & listenPairs = it->getListen();
		if (listenPairs.first == Port && listenPairs.second == IP)
			vect.push_back(*it);
	}
	return vect;
}

////////////////////////////////////////////////////////////////////////////////
///                           INITIALIZATION                                 ///
////////////////////////////////////////////////////////////////////////////////

void	Server::Setup()
{
	std::vector<std::pair<int, std::string> >::iterator	it = _listenVect.begin();
	struct sockaddr_in serv_addr;

	for (; it != _listenVect.end(); it++)
	{
		int	fdSocket = socket(AF_INET, SOCK_STREAM, 0);
		if (fdSocket == -1)
			throw std::runtime_error("Socket creation failed");
		int opt = 1;
		if (setsockopt(fdSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
			throw std::runtime_error("setsockopt failed");
		_fdSocketVect.push_back(fdSocket);
		serv_addr.sin_family = AF_INET;
		serv_addr.sin_port = htons(it->first);
		if (inet_pton(AF_INET, it->second.c_str(), &serv_addr.sin_addr) <= 0)
			throw std::runtime_error("String to IP conversion failed");
		if (bind(fdSocket, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
			throw std::runtime_error("Bind execution failed");
		if (listen(fdSocket, SOMAXCONN) == -1)
			throw std::runtime_error("Listen execution failed");
	}

	std::vector<int>::iterator	itFd = _fdSocketVect.begin();
	for (; itFd != _fdSocketVect.end(); itFd++)
		_pollManager->addSocket(*itFd, POLLIN);
	
}

void	Server::addPair(std::pair<int, std::string> listen)
{
 	std::vector<std::pair<int, std::string> >::iterator	it = std::find(_listenVect.begin(), _listenVect.end(), listen);

	if (it == _listenVect.end())
	{
		_listenVect.push_back(listen);
	}
}

void	Server::supressClient(int fdClient, size_t & i)
{
	std::cout << "\033[35m";
	logTime();
	std::cout << "[INFO]\t\tClient " << _clientsMap[fdClient].getClientPort()
			  << " closed on: "
			  << _clientsMap[fdClient].getIP() << ":"
			  << _clientsMap[fdClient].getLocalPort() << "\033[0m" << std::endl;
	close(fdClient);
	_pollManager->removeSocket(i);
	_clientsMap.erase(fdClient);
	i--;
}

////////////////////////////////////////////////////////////////////////////////
///                                 LOGS                                     ///
////////////////////////////////////////////////////////////////////////////////

void	Server::readLog(int fdClient)
{
	logTime();
	std::cout << "[INFO]\t\tReading from client "
		  << _clientsMap[fdClient].getClientPort()
		  << " on: "
		  << _clientsMap[fdClient].getIP() << ":"
		  << _clientsMap[fdClient].getLocalPort() << std::endl;
}

void	Server::announce() const
{
	std::cout << "\n\n" << "\033[32m";
	logTime();
	std::cout << "[INFO]\t\tServer successfully launched" << "\033[0m" <<std::endl;
}

void	Server::logTime() const
{
	std::time_t	now = std::time(NULL);
	char		timeStamp[20];
	std::strftime(timeStamp, sizeof(timeStamp), "%Y-%m-%d %H:%M:%S", std::localtime(&now));

	std::cout << "[" << timeStamp << "] ";
}

////////////////////////////////////////////////////////////////////////////////
///                              NON MEMBER                                  ///
////////////////////////////////////////////////////////////////////////////////

void strToLower(char *str)
{
	while (*str)
	{
		*str = (char)tolower((unsigned char)*str);
		str++;
	}
}

int getLocalPort(int fd)
{
	struct sockaddr_in addr;
	socklen_t len = sizeof(addr);

	if (getsockname(fd, (struct sockaddr*)&addr, &len) == -1)
		return (-1);

	return ntohs(addr.sin_port);
}




