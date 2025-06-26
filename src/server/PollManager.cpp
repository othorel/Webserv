
#include "../../include/server/PollManager.hpp"

////////////////////////////////////////////////////////////////////////////////
///                               CANONIC +                                  ///
////////////////////////////////////////////////////////////////////////////////

PollManager::PollManager()
{
	struct	pollfd stdIn;
	stdIn.fd = STDIN_FILENO;
	stdIn.events = POLLIN;
	stdIn.revents = 0;
	_fdPollVect.push_back(stdIn);
}

PollManager::~PollManager(){}

PollManager::PollManager(const PollManager & toCopy)
{
	*this = toCopy;
}

PollManager & PollManager::operator=(const PollManager & other)
{
	if (this != &other)
	{
		this->_fdPollVect = other._fdPollVect;
	}
	return (*this);
}

////////////////////////////////////////////////////////////////////////////////
///                                RUNTIME                                   ///
////////////////////////////////////////////////////////////////////////////////

int	PollManager::pollExec(int timeout)
{
	int res = poll(_fdPollVect.data(), _fdPollVect.size(), timeout);
	
    if (res < 0)
        throw std::runtime_error("Poll failed");
    return (res);
}

void	PollManager::addSocket(int fd, short mask)
{
	struct pollfd	newSocket;

	newSocket.fd = fd;
	newSocket.events = mask;
	newSocket.revents = 0;
	_fdPollVect.push_back(newSocket);
}

void	PollManager::removeSocket(int i)
{
	_fdPollVect.erase(_fdPollVect.begin() + i);
}

////////////////////////////////////////////////////////////////////////////////
///                                 GETTERS                                  ///
////////////////////////////////////////////////////////////////////////////////

std::vector<struct pollfd>	PollManager::getPollFdVector(void) const
{
	return (_fdPollVect);
}

////////////////////////////////////////////////////////////////////////////////
///                                 SETTERS                                  ///
////////////////////////////////////////////////////////////////////////////////

void	PollManager::setState(int fd, short mask)
{
	if (mask != POLLIN && mask != POLLOUT)
		return;
	std::vector<struct pollfd>::iterator	it = _fdPollVect.begin();
	while (it != _fdPollVect.end())
	{
		if (it->fd == fd)
			it->events = mask;
		it++;
	}
}

