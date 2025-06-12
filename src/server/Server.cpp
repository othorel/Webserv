
#include "../../include/server/Server.hpp"
#include "../../include/server/PollManager.hpp"
#include "../../include/server/Connexion.hpp"
#include "../../include/config/ServerConfig.hpp"
#include "../../include/config/ConfigParser.hpp"
#include "../../include/http/RequestParser.hpp"
#include "../../include/http/ProcessRequest.hpp"
#include "../../include/http/HttpErrorException.hpp"

////////////////////////////////////////////////////////////////////////////////
///                               CANONIC +                                  ///
////////////////////////////////////////////////////////////////////////////////

Server::Server()
{
	_pollManager = NULL;
}
Server::Server(const ConfigParser & Parser) : _pollManager(new PollManager()), _serverConfigVect(Parser.getServerConfigVector())
{
	std::vector<ServerConfig>::const_iterator	it = _serverConfigVect.begin();
	while (it != _serverConfigVect.end())
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
		this->_pollManager = other._pollManager;
		this->_fdSocketVect = other._fdSocketVect;
		this->_listenVect = other._listenVect;
		this->_activeListenVect = other._activeListenVect;
		this->_clientsMap = other._clientsMap;
		this->_serverConfigVect = other._serverConfigVect;
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

std::vector<std::pair<int, std::string> >	Server::getActiveListenVec() const
{
	return (this->_activeListenVect);
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

std::vector<ServerConfig> 					Server::getServerConfig() const
{
	return (this->_serverConfigVect);
}

////////////////////////////////////////////////////////////////////////////////
///                                 SETTERS                                  ///
////////////////////////////////////////////////////////////////////////////////

void	Server::setServerConfig(std::vector<ServerConfig> & servConfigVect)
{
	this->_serverConfigVect = servConfigVect;
}

////////////////////////////////////////////////////////////////////////////////
///                                RUNTIME                                   ///
////////////////////////////////////////////////////////////////////////////////

void	Server::StartEventLoop()
{
	while (1)
	{
		_pollManager->pollExec(-1);
		fillActiveListenVect();
		for (size_t i = 0; i != _pollManager->getPollFdVector().size(); i++)
		{
			if (_pollManager->getPollFdVector()[i].revents & POLLIN)
				dealClient(_pollManager->getPollFdVector()[i].fd, i);
		}
	}
}

void	Server::dealClient(int fd, size_t & i)
{
	if (std::find(_fdSocketVect.begin(), _fdSocketVect.end(), fd) != _fdSocketVect.end())
	{
		logTime();
		std::cout << "[INFO] New connexion request" << std::endl;
		acceptNewConnexion(fd);
	}
	else
	{
		
		handleEvent(fd, i);
	}
}

void	Server::acceptNewConnexion(int fd)
{
	struct sockaddr_in clientAddr;

	socklen_t addrLen = sizeof(clientAddr);
	int clientFd = accept(fd, (struct sockaddr*)&clientAddr, &addrLen);
	if (clientFd < 0)
		return;
	_pollManager->addSocket(clientFd, POLLIN);
	_clientsMap[clientFd] = Connexion(clientFd, clientAddr, _serverConfigVect);
	std::cout << "[INFO] New connexion authorized on:"
			  << _clientsMap[clientFd].getIP() << ":"
			  << _clientsMap[clientFd].getPort() << std::endl;
}

void	Server::handleEvent(int fdClient, size_t & i)
{
	std::cout << "[INFO] Reading on: "
			  << _clientsMap[fdClient].getIP() << ":"
			  << _clientsMap[fdClient].getPort() << std::endl;

	checkTimeOut(fdClient, i); //on regarde si le timeout est depasse si c'est le cas tout s'arrete et le client est supprime

	std::string		rawLineString;
	_clientsMap[fdClient].readDataFromSocket(rawLineString); // quoi qu'il arrive on lit une ligne sur le socket
	if (_clientsMap[fdClient].getProcessRequest() == NULL)
		_clientsMap[fdClient].setProcessRequest();

	std::string	processed = _clientsMap[fdClient].getProcessRequest()->process(rawLineString);
	while (!processed.empty()) {
		_clientsMap[fdClient].writeDataToSocket(processed);
		processed = _clientsMap[fdClient].getProcessRequest()->process(rawLineString);
	}
	// processed = _clientsMap[fdClient].getProcessRequest().process(rawLineString);

	if (_clientsMap[fdClient].getBytesIn() <= 0) //si on detecte la fermeture de la connexion
	{
		supressClient(fdClient, i); // peut etre en plus throw une exception ?
		if (_clientsMap[fdClient].getBytesIn() < 0)
			throw std::runtime_error("Error while reading from socket");
		return ;
	}
	// else if (_clientsMap[fdClient].endTransmission(rawLineString) == true)
	//{
		// std::cout << "Paquet:\n" << rawLineString << std::endl;
		// std::string	processed = _clientsMap[fdClient].getProcessRequest().process(rawLineString);
		// if (_clientsMap[fdClient].getProcessRequest().getProcessStatus() == WAITING_BODY && _clientsMap[fdClient].getServConfig() == NULL)
		// 	_clientsMap[fdClient].setServConfig(new ServerConfig(_clientsMap[fdClient].getProcessRequest().getServer())); //On initialise le pointeur vers servconfig

		// if (_clientsMap[fdClient].getProcessRequest().getProcessStatus() == SENDING_HEADERS
		// 	|| _clientsMap[fdClient].getProcessRequest().getProcessStatus() == SENDING_BODY) // Si le processRequest a fini de construire la reponse
		// {
		// 	while (! processed.empty())
		// 	{
				// std::cout << "processing" << std::endl;
				// _clientsMap[fdClient].writeDataToSocket(processed);
				// processed = _clientsMap[fdClient].getProcessRequest().process(rawLineString);
		// 	}
		// }
		// else if (_clientsMap[fdClient].getProcessRequest().getProcessStatus() == DONE)
		// 	supressClient(fdClient, i);
	//}
}

void	Server::fillActiveListenVect()
{
	const std::vector<struct pollfd>& pollFds = _pollManager->getPollFdVector();

	for (size_t i = 0; i < pollFds.size(); ++i)
	{
		int fd = pollFds[i].fd;
		if ((pollFds[i].revents & POLLIN) &&
			(std::find(_fdSocketVect.begin(), _fdSocketVect.end(), fd) != _fdSocketVect.end())) // Si l'événement est actif (POLLIN) et que c'est un socket d'écoute
		{
			
			for (size_t j = 0; j < _fdSocketVect.size(); ++j)// On recupere lec couple IP Port associé à ce fd
			{
				if (_fdSocketVect[j] == fd)
				{
					_activeListenVect.push_back(_listenVect[j]);
					break;
				}
			}
		}
	}
}

void	Server::checkTimeOut(int fdClient, size_t & i)
{
	if (_clientsMap[fdClient].getServConfig() == NULL)
		return;
	std::time_t		timeNow = std::time(NULL);
	std::time_t		timeOut = static_cast<time_t>(_clientsMap[fdClient].getServConfig()->getSessionTimeout());
	std::time_t		timeEnd = _clientsMap[fdClient].getStartTime() + timeOut;

	if (timeNow > timeEnd)
	{
		supressClient(fdClient, i);
		throw HttpErrorException(408);
	}
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
	_pollManager->removeSocket(i);
	_clientsMap.erase(fdClient);
	i--;
}

void	Server::logTime() const
{
	std::time_t	now = std::time(NULL);
	char		timeStamp[20];
	std::strftime(timeStamp, sizeof(timeStamp), "%Y-%m-%d %H:%M:%S", std::localtime(&now));

	std::cout << "[" << timeStamp << "] ";
}

