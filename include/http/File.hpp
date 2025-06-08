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

		File & operator=(const File & other);
		File(const File & other);

		bool isExistingFile() const;
		bool isWritableDirectory() const;
		bool isReadableRegularFile() const;
		bool isWritableRegularFile() const;
		off_t getFileSize() const;

	public :

		File();
		File(const std::string & path, bool isWriteMode = false);
		~File();

		const std::string & getPath() const;
		off_t getSize() const;
		int getFd() const;
		size_t getOffset() const;
		bool isWriteMode() const;
		bool isOpen() const;
		void debug() const;

		bool openFile();
		bool closeFile();

		size_t	ReadChunk(char * buffer, size_t readSize);
		size_t	WriteChunk(char * src, size_t writeSize);

};

#endif




// C’est une **excellente décision de conception** 👏. Tu es en train de poser les bases d’un composant **réutilisable, modulaire et solide**, parfaitement adapté à Webserv.

// ---

// ## ✅ Pourquoi une classe `File` générique est une très bonne idée ?

// | Besoin                           | Exemple dans Webserv    | Bénéfice avec `File`        |
// | -------------------------------- | ----------------------- | --------------------------- |
// | Lire un fichier en chunks        | réponse à un `GET`      | ✅ méthode `readChunk()`     |
// | Écrire un fichier en chunks      | traitement d’un `POST`  | ✅ méthode `writeChunk()`    |
// | Savoir si un fichier est ouvert  | sécurité et robustesse  | ✅ méthode `isOpen()`        |
// | Obtenir la taille                | header `Content-Length` | ✅ méthode `getSize()`       |
// | Gérer l’ouverture conditionnelle | lazy open               | ✅ méthode `openIfNeeded()`  |
// | Fermer automatiquement           | RAII                    | ✅ dans le destructeur       |
// | Créer un fichier temporaire      | upload + CGI            | ✅ via `open(path, O_CREAT)` |

// ---

// ## 🏗️ Design proposé : `File`

// ### 🔖 Interface possible (`File.hpp`)

// ```cpp
// #ifndef FILE_HPP
// #define FILE_HPP

// #include <string>
// #include <unistd.h>
// #include <fcntl.h>
// #include <sys/stat.h>

// class File {
// 	private:
// 		std::string _path;
// 		int         _fd;
// 		size_t      _offset;
// 		off_t       _size;
// 		bool        _isWriteMode;

// 	public:
// 		File(); // constructeur par défaut
// 		File(const std::string& path, bool writeMode = false);
// 		~File();

// 		bool openFile();                        // ouvre (lecture ou écriture)
// 		ssize_t readChunk(char *buf, size_t maxSize);
// 		ssize_t writeChunk(const char *buf, size_t len);
// 		void closeFile();

// 		bool isOpen() const;
// 		bool isEOF() const;
// 		off_t getSize() const;
// 		const std::string & getPath() const;
// };

// #endif
// ```

// ---

// ### 🔁 Utilisation possible dans Webserv

// #### 🔹 Pour un `GET` (lecture) :

// ```cpp
// File file("index.html");
// file.openFile();
// char buf[1024];
// ssize_t len = file.readChunk(buf, sizeof(buf));
// ```

// #### 🔹 Pour un `POST` (écriture) :

// ```cpp
// File outFile("upload/received.txt", true); // true = mode écriture
// outFile.openFile();
// outFile.writeChunk(bodyData, bodyLen);
// ```

// ---

// ### 📎 Bonus : gestion des erreurs et exceptions

// Tu pourras plus tard encapsuler les erreurs dans une exception personnalisée `FileIOException` ou retourner `false` si `open()` ou `write()` échouent, à traiter proprement dans les handlers.

// ---

// ### ✅ Résultat

// Tu auras une classe :

// * réutilisable par tous les Handlers (`GetHandler`, `PostHandler`, `CgiHandler`, etc.)
// * compatible avec `poll()` ou `select()` (lecture/écriture non bloquante si le `fd` est configuré)
// * facile à tester en isolation
// * propre et conforme à la **Single Responsibility Principle**

