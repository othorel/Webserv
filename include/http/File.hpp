#ifndef FILE_HPP
# define FILE_HPP

# include <string>
# include <sys/types.h>

class File
{
	private :

		std::string	_path;
		off_t		_size;
		int			_fd;
		size_t		_offset;
		bool		_isWriteMode;

		bool isExistingFile() const;
		bool isWritableDirectory() const;
		bool isReadableRegularFile() const;
		bool isWritableRegularFile() const;
		off_t getFileSize() const;

	public :

		File();
		File(const std::string & path, bool isWriteMode = false);
		File & operator=(const File & other);
		File(const File & other);
		~File();

		const std::string & getPath() const;
		off_t getSize() const;
		int getFd() const;
		size_t getOffset() const;
		std::string getMimeType() const;
		bool isWriteMode() const;
		bool isOpen() const;
		void debug() const;

		bool openFile();
		bool closeFile();

		size_t	ReadChunk(char * buffer, size_t readSize);
		size_t	WriteChunk(const char * src, size_t writeSize);

};

#endif
