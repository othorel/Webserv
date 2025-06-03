
#include "../../include/server/Server.hpp"
#include "../../include/config/ServerConfig.hpp"
#include "../../include/config/ConfigParser.hpp"

Server::Server(){}

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

Server::Server(std::string str)
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

Server::~Server(){}
		
Server & Server::operator=(const Server & other)
{
	if (this != &other)
	{
		this->_fdPollVect = other._fdPollVect;
		this->_fdSocketVect = other._fdSocketVect;
		this->_listenTab = other._listenTab;
	}
	return (*this);
}

void	Server::Run()
{
	while (1)
	{
		if (poll(&_fdPollVect[0], _fdPollVect.size(), -1) == -1)
			throw std::runtime_error("Poll initialization failed");

		for (int i = 0; i != static_cast<int>(_fdPollVect.size()); i++)
		{
			if (_fdPollVect[i].revents & POLLIN)
				dealClient(_fdPollVect[i].fd, i);
		}
	}
}

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
	{
		struct	pollfd	pfd;
		pfd.fd = *itFd;
		pfd.events = POLLIN;
		pfd.revents = 0;
		_fdPollVect.push_back(pfd);
	}
}



void	Server::dealClient(int fd, int & i)
{
	std::cout << "je deal client" << std::endl;
	if (std::find(_fdSocketVect.begin(), _fdSocketVect.end(), fd) != _fdSocketVect.end())
	{
		std::cout << "j add new connexion" << std::endl;
		addNewConnexion(fd);
	}
	else
	{
		std::cout << "je deal existing client" << std::endl;
		dealExistingClient(fd, i);
	}
}

void	Server::addNewConnexion(int fd)
{
	struct sockaddr_in clientAddr;

	socklen_t addrLen = sizeof(clientAddr);
	int clientFd = accept(fd, (struct sockaddr*)&clientAddr, &addrLen);
	if (clientFd < 0)
		return;

	struct pollfd	clientPoll;

	clientPoll.fd = clientFd;
	clientPoll.events = POLLIN;
	clientPoll.revents = 0;
	_fdPollVect.push_back(clientPoll);
}

void	Server::dealExistingClient(int fdClient, int & i)
{
	char		buf[1024];
	ssize_t		bytes = recv(fdClient, buf, sizeof(buf), 0);

	if (bytes <= 0)
	{
		std::cout << "je close mon client" << std::endl;
		close(fdClient);
		_fdPollVect.erase(_fdPollVect.begin() + i);
		i--;
	}
	else
	{
		dealRequest(fdClient);
	}
}

void	Server::addPair(std::pair<int, std::string> listen)
{
 	std::vector<std::pair<int, std::string> >::iterator	it = std::find(_listenTab.begin(), _listenTab.end(), listen);

	if (it == _listenTab.end())
		_listenTab.push_back(listen);
}

void	Server::dealRequest(int fd)
{
	(void)fd;
	std::cout << "test" <<std::endl;
}
