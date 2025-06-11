
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
Server::Server(const ConfigParser & Parser, const std::vector<ServerConfig> servConfigVect) : _pollManager(new PollManager()), _serverConfigVect(servConfigVect)
{
	std::vector<ServerConfig>::const_iterator	it = Parser.getServerConfigVector().begin();
	while (it != Parser.getServerConfigVector().end())
	{
		addPair(it->getListen());
		it++;
	}
	Setup();

}

Server::Server(const std::string str) : _pollManager(new PollManager())
{
	if (str == "debug")
	{
		addPair(std::make_pair(8080, "127.0.0.1"));
		addPair(std::make_pair(8888, "127.0.0.1"));
		Setup();
	}
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
		for (size_t i = 0; i < _activeListenVect.size(); i++)
		{
			std::cout << "Something happens on: " << _activeListenVect[i].second << ":" << _activeListenVect[i].first << std::endl;
		}
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
		acceptNewConnexion(fd);
	else
	{
		std::cout << "New event to handle" << std::endl;
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
	_clientsMap.insert(std::make_pair(clientFd, Connexion(clientFd, clientAddr)));
	std::cout << "New connexion authorized" << std::endl;
}

void	Server::handleEvent(int fdClient, size_t & i)
{
	//checkTimeOut(fdClient, i); //on regarde si le timeout est depasse si c'est le cas tout s'arrete et le client est supprime

	std::string		rawLineString;
	ssize_t			bytesReceived = _clientsMap[fdClient].readDataFromSocket(rawLineString); // quoi qu'il arrive on lit une ligne sur le socket et on l'ajoute a buffer_in
	ssize_t			bytesSend = 0;

	if (bytesReceived <= 0) //si on detecte la fermeture de la connexion
	{
		std::cout << "\nI did not read anything\n" << std::endl;
		delete[] rawLineChar;
		supressClient(fdClient, i); // peut etre en plus throw une exception ?
		return ;
	}
	else
	{
		std::string	processed = _clientsMap[fdClient].getProcessRequest().process();
		while (! processed.empty())
		{

			processed = _clientsMap[fdClient].getProcessRequest().process();
		}

		
	}
}

void	Server::dealRequest(int fd)
{
	const char *response =
		"HTTP/1.1 200 OK\r\n"
		"Content-Type: text/plain\r\n"
		"Content-Length: 13\r\n"
		"\r\n"
		"Hello, world!";

	send(fd, response, strlen(response), 0);
	std::cout << "I just sent a response" << std::endl;
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

const ServerConfig	&Server::getSingleServerConfig(struct sockaddr_in addr) const
{
	for(size_t i = 0; i < _serverConfigVect.size(); i++)
	{
		if (_serverConfigVect[i].getListen().first == ntohs(addr.sin_port) && _serverConfigVect[i].getListen().second == inet_ntoa(addr.sin_addr))
			return (_serverConfigVect[i]);
	}
	throw std::runtime_error("No matching ServConfig for this given sockaddr_in");
}

void	Server::checkTimeOut(int fdClient, size_t & i)
{
	std::time_t		timeNow = std::time(NULL);
	std::time_t		timeOut = static_cast<time_t>(this->getSingleServerConfig(_clientsMap[fdClient].getAddr()).getSessionTimeout());
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

void	Server::parseHeader(int fdClient, std::string &rawrequest)
{
	if (_clientsMap[fdClient].endTransmission() == false) //si on n'a pas encore rencontre de double saut de ligne ca veut dire qu'on est encore dans les headers
		
		_clientsMap[fdClient].appendRaw("HEADER", rawrequest); //alors on ajoute la ligne actuelle a l'attribut _headers
	else
	{
		_clientsMap[fdClient].setHeaderParsed(); //on indique que les headers ont ete parses
		// if (_clientsMap[fdClient].getHttpRequest() == NULL)
		// {
		std::cout << "\nHEADERS PARSED: \n" << _clientsMap[fdClient].getHeaders(); //debug
//
		RequestParser	parser(_clientsMap[fdClient].getHeaders());                      //
		_clientsMap[fdClient].setRequestParser(parser.release());//on initialise _httpRequest

		_clientsMap[fdClient].setProcessRequest(new ProcessRequest(*_clientsMap[fdClient].getHttpRequest(), this->getServerConfig())); // on initialise _processRequest
		// }
	}
}




