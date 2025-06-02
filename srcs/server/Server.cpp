
#include "Server.hpp"
#include "./include/config/ServerConfig.hpp"
#include "./include/config/ConfigParser.hpp"

Server::Server(){}
	
Server::Server(const ConfigParser & servconfig)
{
	std::vector<ServerConfig>::iterator	it = servconfig.getServerConfigVector().begin();
	while (it != servconfig.getServerConfigVector().end())
	{
		addPair(it->getListen());
		it++;
	}
}

Server::Server(const Server & toCopy){}

Server::~Server(){}
		
Server & Server::operator=(const Server & other){}

void	Server::Run()
{
	while (1)
	{
		if (poll(&_fdPollVect[0], _fdPollVect.size(), -1) == -1)
			throw std::runtime_error("Poll initialization failed");

		std::vector<struct pollfd>::iterator	it = _fdPollVect.begin();
		int	i = 0;
		for (; it != _fdPollVect.end(); it++)
		{
			if (it->revents & POLLIN)
				dealClient(it->fd, i);
			i++;
		}
	}
}

void	Server::Setup()
{
	std::vector<std::pair<int, std::string>>::iterator	it = _listenTab.begin();
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
	if (std::find(_fdSocketVect.begin(), _fdSocketVect.end(), fd) != _fdSocketVect.end())
		addNewConnexion(fd);
	else
		dealExistingClient(fd, i);
}

void	Server::addNewConnexion(int fdClient)
{
	struct sockaddr_in clientAddr;

	socklen_t addrLen = sizeof(clientAddr);
	int clientFd = accept(fdClient, (struct sockaddr*)&clientAddr, &addrLen);
	if (clientFd < 0)
		return;

	struct pollfd	clientPoll;

	clientPoll.fd = fdClient;
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
 	std::vector<std::pair<int, std::string>>::iterator	it = std::find(_listenTab.begin(), _listenTab.end(), listen);

	if (it == _listenTab.end())
		_listenTab.push_back(listen);
}

void	Server::dealRequest(int fd)
{
	(void)fd;
}
