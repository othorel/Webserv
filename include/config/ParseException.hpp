#ifndef PARSEEXCEPTION_HPP
# define PARSEEXCEPTION_HPP

# include <exception>
# include <string>

class ParseException : public std::exception {

	private:

		std::string _msg;

	public:

		ParseException(const std::string& msg) : _msg(msg) {}
		virtual ~ParseException() throw() {}
		virtual const char* what() const throw() {return _msg.c_str(); }
};

#endif
