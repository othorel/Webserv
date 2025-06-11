
#include "../../include/server/Connexion.hpp"
#include "../../include/http/RequestParser.hpp"
#include "../../include/http/HttpRequest.hpp"
#include "../../include/http/ProcessRequest.hpp"
#include "../../include/config/Location.hpp"
#include "../../include/config/ServerConfig.hpp"
#include "../../include/http/HttpErrorException.hpp"

////////////////////////////////////////////////////////////////////////////////
///                               CANONIC +                                  ///
////////////////////////////////////////////////////////////////////////////////

Connexion::Connexion(){}

Connexion::Connexion(int fd, sockaddr_in addr, std::vector<ServerConfig> vectServerConfig) : _fd(fd), _addr(addr), _serverConfigVect(vectServerConfig), _processRequest(ProcessRequest(getServConfigVect()))
{
	_startTime = std::time(NULL);
	_servConfig = NULL;
	_bytesIn = 0;
	_bytesOut = 0;
}

Connexion::~Connexion()
{
	if (_servConfig)
		delete _servConfig;
}

Connexion::Connexion(const Connexion & toCopy)
{
	*this = toCopy;
}

Connexion & Connexion::operator=(const Connexion & other)
{
	if (this != &other)
	{
		_fd = other._fd;
		_addr = other._addr;
		_processRequest = other._processRequest;
		_startTime = other._startTime;
		_serverConfigVect = other._serverConfigVect;
		_servConfig = new ServerConfig(*_servConfig);
		_bytesIn = other._bytesIn;
		_bytesOut = other._bytesOut;
		_bufferIn = other._bufferIn;
		_bufferOut = other._bufferOut;
	}
	return (*this);
}

////////////////////////////////////////////////////////////////////////////////
///                                RUNTIME                                   ///
////////////////////////////////////////////////////////////////////////////////

void	Connexion::readDataFromSocket(std::string &line)
{
	char buf[1024];
	memset(buf, 0, 1024);
	_bytesIn = recv(_fd, buf, sizeof(buf), 0);

	if (_bytesIn > 0)
	{
		std::cout << "Parsing request" << std::endl;
		_bufferIn = std::string(buf, _bytesIn);
	}
}

void	Connexion::writeDataToSocket(const std::string & response)
{
	_bytesOut = send(_fd, response.c_str(), response.size(), 0);
}

bool	Connexion::endTransmission()
{
	return (_bufferIn.find("\r\n\r\n") != std::string::npos);
}

////////////////////////////////////////////////////////////////////////////////
///                                 GETTERS                                  ///
////////////////////////////////////////////////////////////////////////////////

int	Connexion::getFd() const
{
	return (_fd);
}

std::string	Connexion::getIP() const
{
	char buf[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &_addr.sin_addr, buf, INET_ADDRSTRLEN);
    return (std::string(buf));
}

int	Connexion::getPort() const
{
	return (ntohs(_addr.sin_port));
}

std::string	Connexion::getBufferIn() const
{
	return (_bufferIn);
}

std::string	Connexion::getBufferOut() const
{
	return (_bufferOut);
}

std::time_t	Connexion::getStartTime() const
{
	return (_startTime);
}

ssize_t	Connexion::getBytesIn() const
{
	return (_bytesIn);
}

ssize_t	Connexion::getBytesOut() const
{
	return (_bytesOut);
}

std::string	Connexion::getBufferIn() const
{
	return (_bufferIn);
}

std::string	Connexion::getBufferOut() const
{
	return (_bufferOut);
}

std::vector<ServerConfig>	Connexion::getServConfigVect() const
{
	return (_serverConfigVect);
}

ServerConfig	*Connexion::getServConfig() const
{
	return (_servConfig);
}

sockaddr_in	Connexion::getAddr() const
{
	return (_addr);
}

ProcessRequest	Connexion::getProcessRequest() const
{
	return (_processRequest);
}

////////////////////////////////////////////////////////////////////////////////
///                                 SETTERS                                  ///
////////////////////////////////////////////////////////////////////////////////

void	Connexion::setBytesIn(ssize_t bytes)
{
	_bytesIn = bytes;
}

void	Connexion::setBytesOut(ssize_t bytes)
{
	_bytesOut = bytes;
}

void	Connexion::setBufferIn(std::string buffer)
{
	_bufferIn = buffer;
}

void	Connexion::setBufferOut(std::string buffer)
{
	_bufferOut = buffer;
}

void	Connexion::setServConfig(ServerConfig *serverconfig)
{
	_servConfig = serverconfig;
}
