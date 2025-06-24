#include <iostream>
#include <sstream>
#include <signal.h>
#include <poll.h>
#include "../../include/cgi/CGIHandler.hpp"
#include "../../include/http/HttpErrorException.hpp"
#include "../../include/http/HttpResponse.hpp"
#include "../../include/http/HttpUtils.hpp"

static void trimWhiteSpaces(std::string & string);
static void stringToLower(std::string & string);

/* ************************************************************************** */
/*                                   constructors                             */
/* ************************************************************************** */

CGIHandler::CGIHandler() :
	_request(),
	_scriptPath(""),
	_queryString(""),
	_response()
{}

CGIHandler::CGIHandler(const HttpRequest & request, const std::string & path) :
	_request(request),
	_scriptPath(path),
	_queryString(""),
	_response()
{
	size_t pos = path.find('?');
	if (pos == std::string::npos)
		_queryString = "";
	else
		_queryString = path.substr(pos + 1);
	_scriptPath = _scriptPath.substr(0, pos);
	buildResponse();
}

CGIHandler::CGIHandler(const CGIHandler & other) :
	_request(other._request),
	_scriptPath(other._scriptPath),
	_queryString(other._queryString),
	_response(other._response)
{}

/* ************************************************************************** */
/*                                    operators                               */
/* ************************************************************************** */

CGIHandler & CGIHandler::operator=(const CGIHandler & other)
{
	if (this != &other) {
		_request = other._request;
		_scriptPath = other._scriptPath;
		_queryString = other._queryString;
		_response = other._response;
	}
	return (*this);
}

/* ************************************************************************** */
/*                                 build response                             */
/* ************************************************************************** */

void CGIHandler::buildResponse()
{
	std::string rawResponse = execute();

	size_t pos = rawResponse.find("\r\n\r\n");
	if (pos == std::string::npos)
		throw HttpErrorException(500, "in CGI: no double endl.");

	std::string headersPart = rawResponse.substr(0, pos);
	std::string body = rawResponse.substr(pos + 4);

	std::map<std::string, std::string> headers;
	std::istringstream headersStream(headersPart);
	std::string line;
	while (std::getline(headersStream, line)) {
		pos = line.find(':');
		if (pos != std::string::npos) {
			if (line[line.size() - 1] == '\r')
				line.erase(line.size() - 1);
			std::string key = line.substr(0, pos);
			trimWhiteSpaces(key);
			stringToLower(key);
			std::string value = line.substr(pos + 1);
			trimWhiteSpaces(value);
			headers[key] = value;
		}
	}
	int status = 200;
	if (headers.find("status") != headers.end())
		status = HttpUtils::stringToInt(headers["status"]);
	_response = HttpResponse(_request.getVersion(), status, headers, body);
}

std::vector<std::string> CGIHandler::buildEnv()
{
	std::vector<std::string> env;

	env.push_back("GATEWAY_INTERFACE=CGI/1.1");
	env.push_back("REQUEST_METHOD=" + _request.getMethod());
	env.push_back("SCRIPT_FILENAME=" + _scriptPath);
	env.push_back("QUERY_STRING=" + _queryString);
	env.push_back("SERVER_PROTOCOL=" + _request.getVersion());
	env.push_back("SERVER_SOFTWARE=MiniWebServ/1.0");
	env.push_back("REDIRECT_STATUS=200");

	std::map<std::string, std::string>::const_iterator it;
	it =_request.getHeaders().find("content-type");
	if (it != _request.getHeaders().end())
		env.push_back("CONTENT_TYPE=" + it->second);
	it =_request.getHeaders().find("content-length");
	if (it != _request.getHeaders().end())
		env.push_back("CONTENT_LENGTH=" + it->second);
	return (env);
}

/* ************************************************************************** */
/*                                     getters                                */
/* ************************************************************************** */

const HttpResponse & CGIHandler::getHttpResponse()
{
	return (_response);
}

/* ************************************************************************** */
/*                              non member functions                          */
/* ************************************************************************** */

static void trimWhiteSpaces(std::string & string)
{
	if (string.empty())
		return ;

	while (!string.empty() && (string[0] == ' ' || string[0] == '\t'))
		string.erase(0, 1);
	while (!string.empty() && (string[string.size() - 1] == ' ' || string[string.size() - 1] == '\t'))
		string.erase(string.size() - 1);
}

static void stringToLower(std::string & string)
{
	for (size_t i = 0; i < string.size(); ++i)
		string[i] = static_cast<char>(std::tolower(string[i]));
}

std::string CGIHandler::execute()
{
	int inputPipe[2];
	int outputPipe[2];
	if (pipe(inputPipe) == -1 || pipe(outputPipe) == -1)
		throw HttpErrorException(500, "in CGI: pipe failed.");
	
	int		timeout_ms = 5000;
	struct	timeval start, now;

	pid_t pid = fork();
	if (pid < 0)
		throw HttpErrorException(500, "in CGI: fork failed.");

	// debug bloc
	// std::cout << "IN CGI SCRIPT :" << std::endl;
	// std::cout << "QUERY STRING : " << _queryString << std::endl;
	// std::cout << "PATH : " << _scriptPath << std::endl;		

	if (pid == 0) {
		dup2(inputPipe[0], STDIN_FILENO);
		dup2(outputPipe[1], STDOUT_FILENO);
		close(inputPipe[1]);
		close(outputPipe[0]);

		std::vector<std::string> envVec = buildEnv();
		std::vector<char*> envp;
		for (size_t i = 0; i < envVec.size(); i++)
			envp.push_back(const_cast<char*>(envVec[i].c_str()));
		envp.push_back(NULL);


	
		char* av[] = {const_cast<char*>(_scriptPath.c_str()), NULL};
		execve(_scriptPath.c_str(), av, &envp[0]);
		perror("execve");
		exit(1);
	}
	else
	{
		int 	status;
		pid_t	resultPid;

		close(inputPipe[0]);
		close(outputPipe[1]);

		if (_request.getMethod() == "POST" && !_request.getBody().empty())
			write(inputPipe[1], _request.getBody().c_str(), _request.getBody().size());
		close(inputPipe[1]);

		char buffer[4096];
		std::string result;
		ssize_t bytes;

		gettimeofday(&start, NULL);
		while (true)
		{
			fd_set readfds;
			FD_ZERO(&readfds);
			FD_SET(outputPipe[0], &readfds);

			gettimeofday(&now, NULL);
			long elapsed = (now.tv_sec - start.tv_sec) * 1000 + (now.tv_usec - start.tv_usec) / 1000;
			long timeout_left = timeout_ms - elapsed;
			if (timeout_left <= 0)
			{
				kill(pid, SIGKILL);
				waitpid(pid, &status, 0);
				throw HttpErrorException(504, "in CGI: timeout exceeded.");
				break;
			}

			struct timeval tv;
			tv.tv_sec = timeout_left / 1000;
			tv.tv_usec = (timeout_left % 1000) * 1000;

			int ready = select(outputPipe[0] + 1, &readfds, NULL, NULL, &tv);
			if (ready > 0)
			{
				bytes = read(outputPipe[0], buffer, sizeof(buffer));
				if (bytes > 0)
					result.append(buffer, bytes);
				else if (bytes == 0)
					break;
				else
				{
					throw HttpErrorException(500, "in CGI: read error.");
					break;
				}
			}
			else if (ready == 0)
			{
				kill(pid, SIGKILL);
				waitpid(pid, &status, 0);
				return (HttpErrorException(500, "in CGI: timeout exceeded.").what());
			}
			else
			{
				throw HttpErrorException(500, "in CGI: select error.");
				break;
			}
		}
		close(outputPipe[0]);
		while (1)
		{
			resultPid = waitpid(pid, &status, WNOHANG);
			if (resultPid != 0)
				break;
			usleep(100000);
		}
		if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
			std::cerr << "CGI script exited abnormally." << std::endl;
			std::cerr << "WIFEXITED = " << WIFEXITED(status) << std::endl;
			std::cerr << "Exit code = " << WEXITSTATUS(status) << std::endl;
			std::cerr << "Script output:\n" << result << std::endl;
			throw HttpErrorException(500, "in CGI: waitpid error.");
		}
		return (result);
	}
}

// std::string CGIHandler::execute()
// {
// 	int inputPipe[2];
// 	int outputPipe[2];
// 	if (pipe(inputPipe) == -1 || pipe(outputPipe) == -1)
// 		throw HttpErrorException(500);

// 	pid_t pid = fork();
// 	if (pid < 0)
// 		throw HttpErrorException(500);

// 	// std::cout << "IN CGI SCRIPT :" << std::endl;
// 	// std::cout << "QUERY STRING : " << _queryString << std::endl;
// 	// std::cout << "PATH : " << _scriptPath << std::endl;		

// 	if (pid == 0) {
// 		dup2(inputPipe[0], STDIN_FILENO);
// 		dup2(outputPipe[1], STDOUT_FILENO);
// 		close(inputPipe[1]);
// 		close(outputPipe[0]);

// 		std::vector<std::string> envVec = buildEnv();
// 		std::vector<char*> envp;
// 		for (size_t i = 0; i < envVec.size(); i++)
// 			envp.push_back(const_cast<char*>(envVec[i].c_str()));
// 		envp.push_back(NULL);

// 		char* av[] = {const_cast<char*>(_scriptPath.c_str()), NULL};
// 		execve(_scriptPath.c_str(), av, &envp[0]);
// 		perror("execve");
// 		exit(1);
// 	}
// 	else {
// 		close(inputPipe[0]);
// 		close(outputPipe[1]);

// 		if (_request.getMethod() == "POST" && !_request.getBody().empty())
// 			write(inputPipe[1], _request.getBody().c_str(), _request.getBody().size());
// 		close(inputPipe[1]);

// 		std::string result;
// 		char buffer[4096];

// 		struct pollfd pfd;
// 		pfd.fd = outputPipe[0];
// 		pfd.events = POLLIN;

// 		int pollStatus = poll(&pfd, 1, 3000); // timeout 3s

// 		if (pollStatus == -1) {
// 			throw HttpErrorException(500);
// 		}
// 		else if (pollStatus == 0) {
// 			kill(pid, SIGKILL);
// 			waitpid(pid, NULL, 0);
// 			throw HttpErrorException(504);
// 		}

// 		ssize_t bytes;
// 		while ((bytes = read(outputPipe[0], buffer, sizeof(buffer))) > 0)
// 			result.append(buffer, bytes);
// 		close(outputPipe[0]);

// 		int status;
// 		waitpid(pid, &status, 0);
// 		if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
// 			std::cerr << "CGI script exited abnormally." << std::endl;
// 			std::cerr << "WIFEXITED = " << WIFEXITED(status) << std::endl;
// 			std::cerr << "Exit code = " << WEXITSTATUS(status) << std::endl;
// 			std::cerr << "Script output:\n" << result << std::endl;
// 			throw HttpErrorException(500);
// 		}
// 		return result;
// 	}
// }