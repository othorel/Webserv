
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

void	Server::Setup()
{
	std::vector<std::pair<int, std::string>>::iterator	it = _listenTab.begin();
	struct sockaddr_in serv_addr;

	for (; it != _listenTab.end(); it++)
	{
		int	fdSocket = socket(AF_INET, SOCK_STREAM, 0);
		if (fdSocket == -1)
			throw ServerSetupException();
		_fdSocketVect.push_back(fdSocket);
		serv_addr.sin_family = AF_INET;
		serv_addr.sin_port = htons(it->first);
		if (inet_pton(AF_INET, it->second.c_str(), &serv_addr.sin_addr) <= 0
			|| bind(fdSocket, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1
			|| listen(fdSocket, SOMAXCONN) == -1)
			throw ServerSetupException();
	}
}

void	Server::addPair(std::pair<int, std::string> listen)
{
 	std::vector<std::pair<int, std::string>>::iterator	it = std::find(_listenTab.begin(), _listenTab.end(), listen);

	if (it == _listenTab.end())
		_listenTab.push_back(listen);
}
