
#include "../../include/server/Server.hpp"
#include "../../include/server/PollManager.hpp"
#include "../../include/server/Connexion.hpp"
#include "../../include/config/ServerConfig.hpp"
// #include "../../include/config/ConfigParser.hpp"

////////////////////////////////////////////////////////////////////////////////
///                               CANONIC +                                  ///
////////////////////////////////////////////////////////////////////////////////

Server::Server()
{
	_pollManager = NULL;
}

// Server::Server(const ConfigParser & servconfig)
// {
// 	std::vector<ServerConfig>::const_iterator	it = servconfig.getServerConfigVector().begin();
// 	while (it != servconfig.getServerConfigVector().end())
// 	{
// 		addPair(it->getListen());
// 		it++;
// 	}
// 	Setup();
// }

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
		// this->_fdPollVect = other._fdPollVect;
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
		// if (poll(&_fdPollVect[0], _fdPollVect.size(), -1) == -1)
		// 	throw std::runtime_error("Poll initialization failed");

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
}

void	Server::handleEvent(int fdClient, int & i)
{
	char		buf[1024];
	ssize_t		bytes = recv(fdClient, buf, sizeof(buf), 0);

	if (bytes <= 0)
	{
		std::cout << "je close mon client" << std::endl;
		close(fdClient);
		_pollManager->removeSocket(i);
		i--;
	}
	else
	{
		dealRequest(fdClient);
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
		_listenTab.push_back(listen);
}
