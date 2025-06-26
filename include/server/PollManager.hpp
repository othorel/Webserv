
#ifndef POLLMANAGER_HPP
# define POLLMANAGER_HPP

# include <iostream>
# include <sys/types.h>
# include <sys/socket.h>
# include <netinet/in.h>
# include <string>
# include <vector>
# include <algorithm>
# include <exception>
# include <arpa/inet.h>
# include <poll.h>
# include <unistd.h>

class PollManager
{
	public:
		PollManager();
		~PollManager();
		PollManager(const PollManager & toCopy);
		PollManager & operator=(const PollManager & other);
	
		//Poll handle
		int							pollExec(int timeout);
		void						addSocket(int fd, short mask);
		void						removeSocket(int i);

		//Getters
		std::vector<struct pollfd>	getPollFdVector(void) const;

		//Setters
		void						setState(int fd, short mask);

	private:
		std::vector<struct pollfd>	_fdPollVect;

};

#endif