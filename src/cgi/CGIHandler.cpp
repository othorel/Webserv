#include "../../include/cgi/CGIHandler.hpp"

CGIHandler::CGIHandler(
	const std::string& scriptPath,
	const std::string& method,
	const std::string& queryString,
	const std::string& body,
	const std::map<std::string, std::string>& headers
) : _scriptPath(scriptPath),
	_method(method),
	_queryString(queryString),
	_body(body),
	_headers(headers)
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

std::string CGIHandler::execute() {
	int inputPipe[2];
	int outputPipe[2];
	if (pipe(inputPipe) == -1 || pipe(outputPipe) == -1)
		throw CGIException("Failed to create pipes");
	pid_t pid = fork();
	if (pid < 0)
		throw CGIException("Fork failed");
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
		std::cerr << "Trying to execve: " << _scriptPath << std::endl;
		execve(_scriptPath.c_str(), av, &envp[0]);
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
			throw CGIException("CGI script execution failed");
		return (result);
	}
}