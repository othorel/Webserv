
#ifndef CONNEXION_HPP
# define CONNEXION_HPP

# include <iostream>
# include <sys/types.h>
# include <sys/socket.h>
# include <netinet/in.h>
# include <string>
# include <vector>
# include <map>
# include <algorithm>
# include <exception>
# include <arpa/inet.h>
# include <poll.h>
# include <unistd.h>

class RequestParser;
class HttpRequest;
class Connexion
{
	public:
		Connexion();
		Connexion(int fd, sockaddr_in addr);
		~Connexion();
		Connexion(const Connexion & toCopy);
		Connexion & operator=(const Connexion & other);

		// Getters
		int				getFd() const;
		std::string		getIP() const;
		int				getPort() const;
		std::string		getBufferIn() const;
		std::string		getBufferOut() const;
		HttpRequest		*getHttpRequest() const;

		// Setters
		void			setRequestParser(HttpRequest	*request);
		// void			setBody();

		// Runtime
		ssize_t			readDataFromSocket();
		ssize_t			writeDataToSocket(const std::string & response);
		bool			isComplete();

	private:
		bool			_isComplete;
		int				_fd;
		sockaddr_in		_addr;
		std::string		_bufferIn;
		std::string		_bufferOut;
		char			*_body;
		HttpRequest		*_request;

};

#endif