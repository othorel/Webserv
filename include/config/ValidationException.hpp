#ifndef VALIDATIONEXCEPTION_HPP
# define VALIDATIONEXCEPTION_HPP

# include <exception>
# include <string>

class ValidationException : public std::exception {

	private:

		std::string _msg;

	public:

		ValidationException(const std::string& msg) : _msg(msg) {}
		virtual ~ValidationException() throw() {}
		virtual const char* what() const throw() {return _msg.c_str(); }
};

#endif
