
#include "../../include/server/Server.hpp"
#include "../../include/server/PollManager.hpp"
#include "../../include/server/Connexion.hpp"
#include "../../include/config/ServerConfig.hpp"
#include "../../include/config/ConfigParser.hpp"
#include "../../include/http/RequestParser.hpp"
#include "../../include/http/ResponseBuilder.hpp"

////////////////////////////////////////////////////////////////////////////////
///                               CANONIC +                                  ///
////////////////////////////////////////////////////////////////////////////////

Server::Server()
{
	_pollManager = NULL;
}

Server::Server(const ConfigParser & servconfig) : _pollManager(new PollManager())
{
	std::vector<ServerConfig>::const_iterator	it = servconfig.getServerConfigVector().begin();
	while (it != servconfig.getServerConfigVector().end())
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
		this->_listenTab = other._listenTab;
	}
	return (*this);
}

////////////////////////////////////////////////////////////////////////////////
///                                RUNTIME                                   ///
////////////////////////////////////////////////////////////////////////////////

void	Server::StartEventLoop()
{
	while (1)
	{
		_pollManager->pollExec(-1);
		for (int i = 0; i != static_cast<int>(_pollManager->getPollFdVector().size()); i++)
		{
			if (_pollManager->getPollFdVector()[i].revents & POLLIN)
				dealClient(_pollManager->getPollFdVector()[i].fd, i);
		}
	}
}

void	Server::dealClient(int fd, int & i)
{
	std::cout << "je deal client" << std::endl;
	if (std::find(_fdSocketVect.begin(), _fdSocketVect.end(), fd) != _fdSocketVect.end())
	{
		std::cout << "j add new connexion" << std::endl;
		acceptNewConnexion(fd);
	}
	else
	{
		std::cout << "je deal existing client" << std::endl;
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
	_clients.insert(std::make_pair(clientFd, Connexion(clientFd, clientAddr)));
	std::cout << "New connexion authorized" << std::endl;
}

void	Server::handleEvent(int fdClient, int & i)
{
	ssize_t	bytes = _clients[fdClient].readDataFromSocket();
	if (bytes <= 0)
	{
		_pollManager->removeSocket(i);
		_clients.erase(fdClient);
		i--;
		return ;
	}
	if (_clients[fdClient].isComplete() == true)
	{
		std::string rawrequest = _clients[fdClient].getBufferIn();
		std::cout << "Raw Request:" << std::endl;
		std::cout << rawrequest << std::endl;

		RequestParser requestparser(rawrequest);
		requestparser.getHttpRequest().debug();

		// ResponseBuilder	responsebuilder(requestparser.getHttpRequest(), );
		// faire un taleau de servs base sur celui de servconfig mois ceux inactifs
		std::string msg = "HTTP/1.1 200 OK\r\nContent-Length: 4\r\n\r\npong";
		_clients[fdClient].writeDataToSocket(msg);
		_pollManager->removeSocket(i);
		_clients.erase(fdClient);
		i--;
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

////////////////////////////////////////////////////////////////////////////////
///                           INITIALIZATION                                 ///
////////////////////////////////////////////////////////////////////////////////

void	Server::Setup()
{
	std::vector<std::pair<int, std::string> >::iterator	it = _listenTab.begin();
	struct sockaddr_in serv_addr;

	for (; it != _listenTab.end(); it++)
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
 	std::vector<std::pair<int, std::string> >::iterator	it = std::find(_listenTab.begin(), _listenTab.end(), listen);

	if (it == _listenTab.end())
	{
		_listenTab.push_back(listen);
	}
}
