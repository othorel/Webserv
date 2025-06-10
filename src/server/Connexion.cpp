
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

// Connexion::Connexion(int fd, sockaddr_in addr, const ServerConfig &servconfig) : _fd(fd), _addr(addr), _request(NULL), _location(NULL), _processRequest(NULL), _servConfig(servconfig)
// {
// 	_endTransmission = false;
// 	_headerIsParsed = false;
// 	_startTime = std::time(NULL);
// }

Connexion::Connexion(int fd, sockaddr_in addr) : _fd(fd), _addr(addr), _request(NULL), _location(NULL), _processRequest(NULL)
{
	_endTransmission = false;
	_headerIsParsed = false;
	_startTime = std::time(NULL);
}

Connexion::~Connexion()
{
	if (_request)
		delete _request;
	if (_location)
		delete _location;
	if (_processRequest)
		delete _processRequest;
}

Connexion::Connexion(const Connexion & toCopy)
{
	*this = toCopy;
}

Connexion & Connexion::operator=(const Connexion & other)
{
	if (this != &other)
	{
		this->_addr = other._addr;
		this->_bufferIn = other._bufferIn;
		this->_bufferOut = other._bufferOut;
		this->_fd = other._fd;
		this->_endTransmission = other._endTransmission;
		this->_body = other._body;
		this->_headerIsParsed = other._headerIsParsed;
		this->_headers = other._headers;
		this->_request = NULL;
		this->_location = NULL;
		this->_processRequest = NULL;
		this->_startTime = other._startTime;
	}
	return (*this);
}

////////////////////////////////////////////////////////////////////////////////
///                                RUNTIME                                   ///
////////////////////////////////////////////////////////////////////////////////

ssize_t	Connexion::readDataFromSocket(std::string &line)
{
	char buf[1024];
	ssize_t bytes = recv(_fd, buf, sizeof(buf), 0);

	if (bytes > 0)
	{
		std::cout << "Parsing request" << std::endl;
		_bufferIn.append(buf, bytes);
		line = buf;
	}

	return (bytes);
}

ssize_t	Connexion::writeDataToSocket(const std::string & response)
{
	return send(_fd, response.c_str(), response.size(), 0);
}

bool	Connexion::endTransmission()
{
	_endTransmission = (_bufferIn.find("\r\n\r\n") != std::string::npos);
	return (_endTransmission);
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

// std::string	Connexion::getBufferOut() const
// {
// 	return (_bufferOut);
// }

HttpRequest	*Connexion::getHttpRequest() const
{
	return (_request);
}

// ServerConfig	Connexion::getServConfig() const
// {
// 	return (_servConfig);
// }

Location	*Connexion::getLocation() const
{
	return (_location);
}

ProcessRequest	*Connexion::getProcessRequest() const
{
	return (_processRequest);
}

bool	Connexion::isHeaderParsed() const
{
	return (_headerIsParsed);
}

std::string		Connexion::getHeaders() const
{
	return (this->_headers);
}

std::string		Connexion::getBody() const
{
	return (this->_body);
}

std::time_t		Connexion::getStartTime() const
{
	return (this->_startTime);
}

const sockaddr_in	&Connexion::getAddr() const
{
	return (_addr);
}

////////////////////////////////////////////////////////////////////////////////
///                                 SETTERS                                  ///
////////////////////////////////////////////////////////////////////////////////

void	Connexion::setRequestParser(HttpRequest	*request)
{
	if (_request)
		delete _request;
 	_request = request;
}

void	Connexion::setHeaderParsed()
{
	_headerIsParsed = true;
}

void	Connexion::setProcessRequest(ProcessRequest *processrequest)
{
	_processRequest = processrequest;
}

void	Connexion::appendRaw(std::string attribute, std::string content)
{
	if (attribute == "HEADER")
	{
		_headers += content;
	}
	else if (attribute == "BODY")
	{
		_body += content;
	}
	else
		throw std::runtime_error("Invalid attribute");
}

static Location *findMatchinglocation(
		const std::map<std::string, Location> & locations,
		const std::string & target)
{
	std::string bestMatch = "";
	std::map<std::string, Location>::const_iterator it;
	for (it = locations.begin(); it != locations.end(); ++it)
	{
		const std::string& path = it->first;
		if (target.compare(0, path.size(), path) == 0)
		{
			if (path.size() > bestMatch.size())
				bestMatch = path;
		}
	}
	if (bestMatch.empty())
		throw HttpErrorException(404);
	
	return (new Location(locations.find(bestMatch)->second));
}