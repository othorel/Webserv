#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <map>
#include "../../include/http/File.hpp"
#include "../../include/http/HttpErrorException.hpp"
#include "../../include/http/HttpUtils.hpp"

/* ************************************************************************** */
/*                                   constructors                             */
/* ************************************************************************** */

File::File() :
	_path(""),
	_name(""),
	_size(-1),
	_fd(-1),
	_offset(0),
	_isWriteMode(false),
	_boundary(""),
	_buffer(""),
	_writeStatus(IN_BODY)
{}

File::File(const std::string & path, bool isWriteMode) :
	_path(path),
	_name(""),
	_size(-1),
	_fd(-1),
	_offset(0),
	_isWriteMode(isWriteMode),
	_boundary(""),
	_buffer(""),
	_writeStatus(IN_BODY)
{
	
	if (!_isWriteMode) {
		if (!isExistingFile())
			throw HttpErrorException(404);
		if (!isReadableRegularFile())
			throw HttpErrorException(403);
		_size = getFileSize();
		if (_size < 0)
			throw (HttpErrorException(500));
	}
	else {
		if (!isWritableDirectory())
			throw HttpErrorException(403);
		if (isExistingFile()) {
			if (!isWritableRegularFile())
				throw HttpErrorException(403);
			_size = getFileSize();
			if (_size < 0)
				throw HttpErrorException(500);
		}
		else
			_size = 0;
	}
}

File::File(const std::string & path, const std::string & boundary) :
	_path(path),
	_name(""),
	_size(-1),
	_fd(-1),
	_offset(0),
	_isWriteMode(true),
	_boundary(boundary),
	_buffer(""),
	_writeStatus(BEFORE_FIRST_BOUNDARY)
{
		if (!isWritableDirectory())
			throw HttpErrorException(403);
		if (isExistingFile()) {
			if (!isWritableRegularFile())
				throw HttpErrorException(403);
			_size = getFileSize();
			if (_size < 0)
				throw HttpErrorException(500);
		}
		else
			_size = 0;


	// debug
	std::cout << "AT FILE CONSTRUCTION, BOUNDARY= " << _boundary << std::endl;
}

File::File(const File & other) :
	_path(other._path),
	_name(other._name),
	_size(other._size),
	_fd(-1),
	_offset(other._offset),
	_isWriteMode(other._isWriteMode),
	_boundary(other._boundary),
	_buffer(other._buffer),
	_writeStatus(other._writeStatus)

{}

/* ************************************************************************** */
/*                                    operators                               */
/* ************************************************************************** */

File & File::operator=(const File & other)
{
	if (this != &other) {
		if (_fd != -1)
			close(_fd);
		_path = other._path;
		_name = other._name;
		_size = other._size;
		_fd = -1;
		_offset = other._offset;
		_isWriteMode = other._isWriteMode;
		_boundary = other._boundary;
		_buffer = other._buffer;
		_writeStatus = other._writeStatus;
	}
	return (*this);
}

/* ************************************************************************** */
/*                                    destructor                              */
/* ************************************************************************** */

File::~File()
{
	if (_fd >= 0) {
		if (close(_fd) == -1)
			std::cerr << "Warning : Unable to close file : " << _path << std::endl;
		else
			_fd = -1;}
}

/* ************************************************************************** */
/*                                     getters                                */
/* ************************************************************************** */

const std::string & File::getPath() const
{
	return (_path);
}

off_t File::getSize() const
{
	return (_size);
}

int File::getFd() const
{
	return (_fd);
}

size_t File::getOffset() const
{
	return (_offset);
}

bool File::isWriteMode() const
{
	return (_isWriteMode);
}

bool File::isOpen() const
{
	return (_fd >= 0);
}

std::string File::getMimeType() const
{
	size_t dot = _path.rfind('.');
	if (dot == std::string::npos) {
		return ("application/octet-stream"); }
	std::string extension = _path.substr(dot +1);
	HttpUtils::stringToLower(extension);

	static std::map<std::string, std::string> extensionMap;
	if (extensionMap.empty()) {
		extensionMap["html"] = "text/html";
		extensionMap["htm"] = "text/html";
		extensionMap["css"] = "text/css";
		extensionMap["txt"] = "text/plain";
		extensionMap["js"] = "application/javascript";
		extensionMap["jpg"] = "image/jpeg";
		extensionMap["jpeg"] = "image/jpeg";
		extensionMap["png"] = "image/png";
		extensionMap["gif"] = "image/gif";
		extensionMap["ico"] = "image/x-icon";
		extensionMap["json"] = "application/x-json";
		extensionMap["pdf"] = "application/pdf";
	}
	if (extensionMap.find(extension) != extensionMap.end()) {
		return (extensionMap[extension]); }
	return ("application/octet-stream");
}

/* ************************************************************************** */
/*                                 open / close                               */
/* ************************************************************************** */

bool File::openFile()
{
	if (isOpen())
		return (true);

	int flags;
	if (_isWriteMode) {
		if (!isWritableDirectory())
			throw HttpErrorException(403);
		flags = O_WRONLY | O_CREAT | O_APPEND | O_NONBLOCK;
	}
	else {
		if (!isExistingFile())
			throw HttpErrorException(404);
		if (!isReadableRegularFile())
			throw HttpErrorException(403);

		_size = getFileSize();
		if (_size < 0)
			throw HttpErrorException(500);
		flags = O_RDONLY | O_NONBLOCK;
	}
	_fd = open(_path.c_str(), flags, 0644);
	if (_fd == -1)
		throw HttpErrorException(500);
	return (true);
}

bool File::closeFile()
{
	if (isOpen()) {
		if (close(_fd) == -1)
			return (false);
		_fd = -1; }
	return (true);
}

/* ************************************************************************** */
/*                         read / write in buffer                             */
/* ************************************************************************** */

size_t	File::ReadChunk(char * buffer, size_t readSize)
{
	if (_isWriteMode)
		throw HttpErrorException(500);
	if (!isOpen()) {
		if (!openFile())
			throw HttpErrorException(500);}
	ssize_t bytesRead = read(_fd, buffer, readSize);
	if (bytesRead < 0)
		throw HttpErrorException(500);
	_offset += static_cast<size_t>(bytesRead);
	return (static_cast<size_t>(bytesRead));
}

size_t	File::WriteChunk(const char * src, size_t writeSize)
{
	if (!_isWriteMode)
		throw HttpErrorException(500);
	if (!isOpen()) {
		if (!openFile())
			throw HttpErrorException(500);}
	if (!_boundary.empty())
		return (writeChunkBoundary(src, writeSize));
	ssize_t bytesWritten = write(_fd, src, writeSize);
	if (bytesWritten < 0)
		throw HttpErrorException(500);
	_offset += static_cast<size_t>(bytesWritten);
	_size += static_cast<size_t>(bytesWritten);
	return (static_cast<size_t>(bytesWritten));
}

size_t File::writeChunkBoundary(const char * src, size_t writeSize)
{
	_buffer.append(src, writeSize);

	if (_writeStatus == BEFORE_FIRST_BOUNDARY) {
		size_t pos = _buffer.find(_boundary);
		if (pos == std::string::npos)
			return (writeSize);
		_buffer = _buffer.substr(pos + _boundary.size());
		_writeStatus = IN_FIRST_BOUNDARY;
	}

	if (_writeStatus == IN_FIRST_BOUNDARY) {
		//debug
		std::cout << "BUFFER = " << _buffer << std::endl;

		// dev
		size_t namePos = _buffer.find("filename=");
		

		size_t pos = _buffer.find("\r\n\r\n");
		if (pos == std::string::npos)
			return (writeSize);
		_buffer = _buffer.substr(pos + 4);
		_writeStatus = IN_BODY;
	}

	if (_writeStatus == IN_BODY) {
		ssize_t bytesWritten = write(_fd, _buffer.c_str(), _buffer.size());
		if (bytesWritten < 0)
			throw HttpErrorException(500);
		_offset += static_cast<size_t>(bytesWritten);
		_size += static_cast<size_t>(bytesWritten);
		_buffer.clear();
	}

	return (writeSize);



	// if (_writeStatus < IN_BODY) {
	// 	size_t	i = 0;
	// 	while (i < writeSize) {
	// 		if (_writeStatus != IN_FIRT_BOUNDARY) {
	// 			if (src[i] == _boundary[_buffer.size()]) {
	// 				_buffer += src[i];
	// 				std::cout << "BOUNDARY BUFFER = " << _buffer << std::endl;
	// 				_inBoundary = true;
	// 			}
	// 		}
	// 		else {
	// 			if (src[i] == _boundary[_buffer.size()]) {
	// 				_buffer += src[i];
	// 				std::cout << "BOUNDARY BUFFER = " << _buffer << std::endl;
	// 				if (_buffer == _boundary) {
	// 					_inBoundary = false;
	// 					_inBody = true;
	// 					break ;
	// 				}
	// 			}
	// 			else {
	// 				_buffer.clear();
	// 				_inBoundary = false;
	// 			}
	// 		}
	// 		i++;
	// 	}
	// }

}

/* ************************************************************************** */
/*                         display the whole object                           */
/* ************************************************************************** */

void File::debug() const
{
	std::cout << "===== File Debug Info =====" << std::endl;
	std::cout << "Path         : " << _path << std::endl;
	std::cout << "Size         : " << _size << std::endl;
	std::cout << "File FD      : " << _fd << (_fd >= 0 ? " (open)" : " (closed)") << std::endl;
	std::cout << "Offset       : " << _offset << std::endl;
	std::cout << "Mode         : " << (_isWriteMode ? "Write" : "Read") << std::endl;
	std::cout << "Is open?     : " << (isOpen() ? "Yes" : "No") << std::endl;
	std::cout << "============================" << std::endl;
}

/* ************************************************************************** */
/*                              private methods                               */
/* ************************************************************************** */

bool File::isExistingFile() const
{
	struct stat s;
	return (stat(_path.c_str(), &s) == 0 && S_ISREG(s.st_mode));
}

bool File::isWritableDirectory() const
{
	std::string dirPath = _path.substr(0, _path.find_last_of('/'));
	if (dirPath.empty())
		dirPath = ".";

	struct stat s;
	if (stat(dirPath.c_str(), &s) != 0)
		return (false);
	if (!S_ISDIR(s.st_mode))
		return (false);
	return (access(dirPath.c_str(), W_OK) == 0);
}

bool File::isReadableRegularFile() const
{
	struct stat s;
	if (stat(_path.c_str(), &s) != 0)
		return (false);
	if (!S_ISREG(s.st_mode))
		return (false);
	return (access(_path.c_str(), R_OK) == 0);
}

bool File::isWritableRegularFile() const
{
	struct stat s;
	if (stat(_path.c_str(), &s) != 0)
		return (false);
	if (!S_ISREG(s.st_mode))
		return (false);
	return (access(_path.c_str(), W_OK) == 0);
}

off_t File::getFileSize() const
{
	struct stat s;
	if (stat(_path.c_str(), &s) == 0 && S_ISREG(s.st_mode))
		return (s.st_size);
	return (-1);
}

/* ************************************************************************** */
/*                                  sanitizer                                 */
/* ************************************************************************** */

// void File::sanitizeMultipart(const std::string & boundary)
// {
// 	const std::string tempPath = _path + ".cleaned";

// 	std::ifstream in(_path.c_str(), std::ios::binary);
// 	std::ofstream out(tempPath.c_str(), std::ios::binary | std::ios::trunc);
// 	if (!in || !out)
// 		throw HttpErrorException(500);

// 	std::string line;
// 	bool inFileContent = false;

// 	while (std::getline(in, line)) {
// 		// Remettre le \n perdu par getline (utile pour binaires encodés)
// 		line += '\n';

// 		if (!inFileContent) {
// 			// On attend la ligne vide après les headers du part
// 			if (line == "\r\n" || line == "\n")
// 				inFileContent = true;
// 			continue; // ne rien écrire tant qu'on n'est pas dans le contenu
// 		}

// 		// Vérifie si on atteint le boundary final
// 		if (line.find("--" + boundary) != std::string::npos)
// 			break;

// 		// Écrit la ligne dans le fichier nettoyé
// 		out.write(line.c_str(), line.size());
// 	}

// 	in.close();
// 	out.close();

// 	// Remplace l'ancien fichier par le fichier nettoyé
// 	if (std::remove(_path.c_str()) != 0)
// 		throw HttpErrorException(500);
// 	if (std::rename(tempPath.c_str(), _path.c_str()) != 0)
// 		throw HttpErrorException(500);

// 	// Met à jour la taille
// 	_size = getFileSize();
// 	_offset = _size;
// }
