#ifndef REQUETHANDLERS_HPP
# define REQUETHANDLERS_HPP

# include <string>
# include <map>

class PocessRequest;

class RequestHandlers
{
	public :

		typedef void (*HandlerFunction)();

	private :

		static std::map<std::string, HandlerFunction> _methodList;

		RequestHandlers();
		RequestHandlers(const RequestHandlers & other);
		RequestHandlers & operator=(const RequestHandlers & other);
		~RequestHandlers();

		static void getHandler(ProcessRequest * req);
		static void postHandler(ProcessRequest * req);
		static void deleteHandler(ProcessRequest * req);
		void ProcessRequest::checkMethodValidity();
		std::string ProcessRequest::createPath();
		std::string ProcessRequest::createUploadPath();
		void ProcessRequest::checkPostValidity(const std::string & path);

	public :

		static HandlerFunction selectHandler(const std::string & method);

};

#endif
