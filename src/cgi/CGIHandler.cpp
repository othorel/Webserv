#include <iostream>
#include "../../include/cgi/CGIHandler.hpp"
#include "../../include/http/HttpErrorException.hpp"

CGIHandler::CGIHandler(const std::string & path, const HttpRequest & request) :
	_scriptPath(path),
	_method(request.getMethod()),
	_target(request.getTarget()),
	_version(request.getVersion()),
	_body(request.getBody()),
	_queryString(""),
	_headers(request.getHeaders())
{}

// a refaire
CGIHandler::CGIHandler() :
	_scriptPath(""),
	_method(""),
	_target(""),
	_version(""),
	_body(""),
	_queryString(""),
	_headers()
{}

std::vector<std::string> CGIHandler::buildEnv() const {
	std::vector<std::string> env;

	env.push_back("GATEWAY_INTERFACE=CGI/1.1");
	env.push_back("REQUEST_METHOD=" + _method);
	env.push_back("SCRIPT_FILENAME=" + _scriptPath);
	env.push_back("QUERY_STRING=" + _queryString);
	env.push_back("SERVER_PROTOCOL=HTTP/1.1");
	env.push_back("SERVER_SOFTWARE=MiniWebServ/1.0");

	std::map<std::string, std::string>::const_iterator it;
	it =_headers.find("Content-Type");
	if (it != _headers.end())
		env.push_back("CONTENT_TYPE=" + it->second);
	it =_headers.find("Content-Length");
	if (it != _headers.end())
		env.push_back("CONTENT_LENGTH=" + it->second);
	return (env);
}

// std::string CGIHandler::execute() {
// 	int inputPipe[2];
// 	int outputPipe[2];
// 	if (pipe(inputPipe) == -1 || pipe(outputPipe) == -1)
// 		throw CGIException("Failed to create pipes");
// 	pid_t pid = fork();
// 	if (pid < 0)
// 		throw CGIException("Fork failed");
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
// 		std::cerr << "Trying to execve: " << _scriptPath << std::endl;
// 		execve(_scriptPath.c_str(), av, &envp[0]);
// 		perror("execve");
// 		exit(1);
// 	}
// 	else {
// 		close(inputPipe[0]);
// 		close(outputPipe[1]);
// 		if (_method == "POST" && !_body.empty())
// 			write(inputPipe[1], _body.c_str(), _body.size());
// 		close(inputPipe[1]);
// 		char buffer[4096];
// 		std::string result;
// 		ssize_t bytes;
// 		while ((bytes = read(outputPipe[0], buffer, sizeof(buffer))) > 0)
// 			result.append(buffer, bytes);
// 		close(outputPipe[0]);
// 		int status;
// 		waitpid(pid, &status, 0);
// 		if (!WIFEXITED(status) || WEXITSTATUS(status) != 0)
// 			throw CGIException("CGI script execution failed");
// 		return (result);
// 	}
// }

std::string CGIHandler::execute()
{
	size_t pos = _target.find('?');
	if (pos == std::string::npos)
		_queryString = "";
	else
		_queryString = _target.substr(pos + 1);


	int inputPipe[2];
	int outputPipe[2];
	if (pipe(inputPipe) == -1 || pipe(outputPipe) == -1)
		throw HttpErrorException(500);

	pid_t pid = fork();
	if (pid < 0)
		throw HttpErrorException(500);

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

		// Détermination de l'interpréteur
		std::string extension = _scriptPath.substr(_scriptPath.find_last_of('.'));
		std::map<std::string, std::string> interpreters;
		interpreters[".py"]  = "/usr/bin/python3";
		interpreters[".pl"]  = "/usr/bin/perl";
		interpreters[".php"] = "/usr/bin/php-cgi";

		std::string interpreter;
		std::map<std::string, std::string>::const_iterator it = interpreters.find(extension);
		if (it != interpreters.end())
			interpreter = it->second;
		else
			throw HttpErrorException(500);

		std::cout << "QUERY STRING : " << _queryString << std::endl;
		std::cout << "INTERPRETEUR : " << interpreter << std::endl;
		std::cout << "PATH : " << _scriptPath << std::endl;

		char* av[] = {
			const_cast<char*>(interpreter.c_str()),
			const_cast<char*>(_scriptPath.c_str()),
			NULL
		};

		std::cerr << "Trying to execve: " << interpreter << " " << _scriptPath << std::endl;
		execve(interpreter.c_str(), av, &envp[0]);
		perror("execve");
		exit(1);
	}
	else {
		close(inputPipe[0]);
		close(outputPipe[1]);
		if (_method == "POST" && !_body.empty())
			write(inputPipe[1], _body.c_str(), _body.size());
		close(inputPipe[1]);

		char buffer[4096];
		std::string result;
		ssize_t bytes;
		while ((bytes = read(outputPipe[0], buffer, sizeof(buffer))) > 0)
			result.append(buffer, bytes);
		close(outputPipe[0]);

		int status;
		waitpid(pid, &status, 0);
		if (!WIFEXITED(status) || WEXITSTATUS(status) != 0)
			throw HttpErrorException(500);

		return result;
	}
}

