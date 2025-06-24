#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <map>
#include "../../include/process/File.hpp"
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
			throw HttpErrorException(404, "in F: Invalid path.");
		if (!isReadableRegularFile())
			throw HttpErrorException(403, "in F: Path is not a regular file.");
		_size = getFileSize();
		if (_size < 0)
			throw (HttpErrorException(500, "in F: Invalid file size."));
	}
	else {
		if (!isWritableDirectory())
			throw HttpErrorException(403, "in F: Path is not a writable directory.");
		if (isExistingFile()) {
			if (!isWritableRegularFile())
				throw HttpErrorException(403, "in F: Path is not a writable regular file.");
			_size = getFileSize();
			if (_size < 0)
				throw HttpErrorException(500, "in F: Invalid file size.");
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
			throw HttpErrorException(403, "in F: Path is not a writable directory.");
		if (isExistingFile()) {
			if (!isWritableRegularFile())
				throw HttpErrorException(403, "in F: Path is not a regular writable file.");
			_size = getFileSize();
			if (_size < 0)
				throw HttpErrorException(500, "in F: Invalid file size.");
		}
		else
			_size = 0;
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

const std::string & File::getName() const
{
	return(_name);
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
	if (dot == std::string::npos)
		return ("application/octet-stream");
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
			throw HttpErrorException(403, "in F: Path is not a writable directory.");
		flags = O_WRONLY | O_CREAT | O_APPEND | O_NONBLOCK;
	}
	else {
		if (!isExistingFile())
			throw HttpErrorException(404, "in F: File does not exists.");
		if (!isReadableRegularFile())
			throw HttpErrorException(403, "in F: Path is not a regular file.");

		_size = getFileSize();
		if (_size < 0)
			throw HttpErrorException(50, "in F: Invalid file size.");
		flags = O_RDONLY | O_NONBLOCK;
	}
	_fd = open(_path.c_str(), flags, 0644);
	if (_fd == -1)
		throw HttpErrorException(500, "in F: Unable to open file.");
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
		throw HttpErrorException(500, "in F: File is in write mode.");
	if (!isOpen()) {
		if (!openFile())
			throw HttpErrorException(500, "in F: Unable to open file.");
	}
	ssize_t bytesRead = read(_fd, buffer, readSize);
	if (bytesRead < 0)
		throw HttpErrorException(500, "in F: No bytes to read.");
	_offset += static_cast<size_t>(bytesRead);
	return (static_cast<size_t>(bytesRead));
}

size_t	File::WriteChunk(const char * src, size_t writeSize)
{
	if (!_isWriteMode)
		throw HttpErrorException(500, "in F: File is not in write mode.");
	if (!isOpen()) {
		if (!openFile())
			throw HttpErrorException(500, "in F: Unable to open file.");
	}
	if (!_boundary.empty())
		return (writeChunkBoundary(src, writeSize));
	ssize_t bytesWritten = write(_fd, src, writeSize);
	if (bytesWritten < 0)
		throw HttpErrorException(500, "in F: Error while writing bytes.");
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
		size_t namePos = _buffer.find("filename=");
		if (namePos != std::string::npos) {
			size_t start = namePos + 9;
			if (_buffer[start] == '\"') ++start;
			size_t end = _buffer.find_first_of("\"\n\r;", start);
			if (end != std::string::npos) {
				_name = _buffer.substr(start, end - start);
			}
		}

		size_t pos = _buffer.find("\r\n\r\n");
		if (pos == std::string::npos)
			return (writeSize);
		_buffer = _buffer.substr(pos + 4);
		_writeStatus = IN_BODY;
	}

	if (_writeStatus == IN_BODY) {
		ssize_t bytesWritten = write(_fd, _buffer.c_str(), _buffer.size());
		if (bytesWritten < 0)
			throw HttpErrorException(500, "in F: Error while writtin bytes.");
		_offset += static_cast<size_t>(bytesWritten);
		_size += static_cast<size_t>(bytesWritten);
		_buffer.clear();
	}

	return (writeSize);
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
