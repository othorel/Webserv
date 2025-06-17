
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

Connexion::Connexion(int fd, sockaddr_in addr, const std::vector<ServerConfig> *_serverConfigVect) :
_fd(fd),
_addr(addr),
_servConfig(NULL),
_endPreviousRequest(0),
_nbRequests(0),
keepAliveMaxRequests(0),
keepAliveTimeOut(0),
_processRequest(*_serverConfigVect)
{
	_startTime = std::time(NULL);
	_bytesIn = 0;
	_bytesOut = 0;
}

Connexion::~Connexion()
{

}

Connexion::Connexion(const Connexion & toCopy)
{
	*this = toCopy;
}

Connexion &Connexion::operator=(const Connexion & other)
{
	if (this != &other)
	{
		this->_fd = other._fd;
		this->_addr = other._addr;
		this->_startTime = other._startTime;
		this->_bytesIn = other._bytesIn;
		this->_bytesOut = other._bytesOut;
		this->_bufferIn = other._bufferIn;
		this->_bufferOut = other._bufferOut;
		this->_processRequest = other._processRequest;
		this->_servConfig = other._servConfig;
		this->_endPreviousRequest = other._endPreviousRequest;
		this->_nbRequests = other._nbRequests;
	}
	return (*this);
}

////////////////////////////////////////////////////////////////////////////////
///                                RUNTIME                                   ///
////////////////////////////////////////////////////////////////////////////////

void	Connexion::readDataFromSocket(std::string &line)
{
	char bufIn[BUFFER_SIZE];
	memset(bufIn, 0, BUFFER_SIZE);
	_bytesIn = recv(_fd, bufIn, sizeof(bufIn), 0);

	if (_bytesIn <= 0)
		return;
		
	// _bufferIn.append(bufIn, _bytesIn);
	_bufferIn = bufIn;
	line = bufIn;
	
	// std::size_t pos = _bufferIn.find("\r\n\r\n");
	// if (pos != std::string::npos)
	// {
	// 	line = _bufferIn.substr(0, pos + 4); //je recupere tout jusqu'a la fin du paquet
	// 	_bufferIn.erase(0, pos + 4); //si il y avait des caracteres apres la fin du paquet je les garde pour la prochaine lecture
	// }
}

void	Connexion::writeDataToSocket(const std::string & response)
{
	ssize_t		totalSent = 0;
	ssize_t		toSend = 0;
	ssize_t		sent = 0;
	while (static_cast<size_t>(totalSent) < response.size())
	{
		toSend = std::min(static_cast<size_t>(BUFFER_SIZE), response.size() - totalSent);
		sent = send(_fd, response.c_str() + totalSent, toSend, 0);
		if (sent == -1)
		{
			_bytesOut = -1;
			return;
		}
		totalSent += sent;
	}
	_bytesOut = totalSent;
}

bool	Connexion::endTransmission(std::string line)
{
	return (line.find("\r\n\r\n") != std::string::npos);
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

int	Connexion::getClientPort() const
{
	return (ntohs(_addr.sin_port));
}

int Connexion::getLocalPort() const
{
    struct sockaddr_in local_addr;
    socklen_t len = sizeof(local_addr);
    if (getsockname(_fd, (struct sockaddr*)&local_addr, &len) == -1)
        return -1; // ou gère l'erreur comme tu veux
    return ntohs(local_addr.sin_port);
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

const ServerConfig	*Connexion::getServConfig() const
{
	return (_servConfig);
}

sockaddr_in	Connexion::getAddr() const
{
	return (_addr);
}

ProcessRequest	&Connexion::getProcessRequest()
{
	return (_processRequest);
}

const ProcessRequest	&Connexion::getProcessRequest() const
{
	return (_processRequest);
}

int	Connexion::getEndPreviousRequest() const
{
	return (_endPreviousRequest);
}

int	Connexion::getNbRequests() const
{
	return (_nbRequests);
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

void	Connexion::setServConfig(const ServerConfig *serverconfig)
{
	_servConfig = serverconfig;
}

void	Connexion::increaseNbRequests()
{
	_nbRequests++;
}

void	Connexion::actualizeEndPreviousRequest()
{
	_endPreviousRequest = std::time(NULL);
}
