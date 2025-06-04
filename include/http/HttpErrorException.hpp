#ifndef HTTPERROREXCEPTION_HPP
# define HTTPERROREXCEPTION_HPP

# include <exception>

class HttpErrorException : public std::exception
{
	private:
		int _statusCode;
	public:
		HttpErrorException(int code) :
			_statusCode(code)
		{}
		int getStatusCode() const
		{
			return (_statusCode);
		}
		const char* what() const throw()
		{
			return ("HTTP Error Exception");
		}
	};

#endif
