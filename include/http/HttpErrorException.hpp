#ifndef HTTPERROREXCEPTION_HPP
# define HTTPERROREXCEPTION_HPP

# include <exception>
# include <string>

class HttpErrorException : public std::exception
{
	private:

		int			_statusCode;
		std::string	_message;

	public:

		virtual ~HttpErrorException() throw() {};

		HttpErrorException(int code, std::string message) :
			_statusCode(code),
			_message(message)
		{}

		int getStatusCode() const
		{
			return (_statusCode);
		}

		const char* what() const throw()
		{
			return (_message.c_str());
		}

};

#endif
