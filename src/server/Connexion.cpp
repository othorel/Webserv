
#include "../../include/server/Connexion.hpp"
#include "../../include/http/RequestParser.hpp"
#include "../../include/http/HttpRequest.hpp"

////////////////////////////////////////////////////////////////////////////////
///                               CANONIC +                                  ///
////////////////////////////////////////////////////////////////////////////////

Connexion::Connexion() : _request(NULL), _body(NULL) {}

Connexion::Connexion(int fd, sockaddr_in addr) : _fd(fd), _addr(addr), _request(NULL), _body(NULL)
{
	_isComplete = false;
}

Connexion::~Connexion()
{
	if (_request)
		delete _request;
	if (_body)
		delete _body;
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
		this->_isComplete = other._isComplete;
		this->_request = other._request;
		this->_body = other._body;
	}
	return (*this);
}


////////////////////////////////////////////////////////////////////////////////
///                                RUNTIME                                   ///
////////////////////////////////////////////////////////////////////////////////

ssize_t	Connexion::readDataFromSocket()
{
	char buf[1024];
	ssize_t bytes = recv(_fd, buf, sizeof(buf), 0);

	if (bytes > 0)
	{
		std::cout << "Parsing request" << std::endl;
		_bufferIn.append(buf, bytes);
	}

	return (bytes);
}

ssize_t	Connexion::writeDataToSocket(const std::string & response)
{
	return send(_fd, response.c_str(), response.size(), 0);
}

bool	Connexion::isComplete()
{
	_isComplete = (_bufferIn.find("\r\n\r\n") != std::string::npos);
	return (_isComplete);
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

HttpRequest	*Connexion::getHttpRequest() const
{
	return (_request);
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

// void	Connexion::setBody()
// {
// 	while (_bufferIn.c_str()[i])
// }
		