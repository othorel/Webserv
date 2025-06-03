#ifndef RESPONSE_BUILDER
# define RESPONSE_BUILDER

# include <exception>
# include "../../include/http/HttpRequest.hpp"
# include "../../include/http/HttpResponse.hpp"
# include "../../include/http/Route.hpp"

class ResponseBuilder
{
	private :

		HttpResponse _httpResponse;

	public :

		ResponseBuilder();
		ResponseBuilder(const HttpRequest& request, const Route& route);
		ResponseBuilder(const ResponseBuilder & other);
		ResponseBuilder & operator=(const ResponseBuilder & other);
		~ResponseBuilder();

		HttpResponse buildResponse(const HttpRequest& request, const Route& route);
		HttpResponse buildError(int statusCode);
		const HttpResponse & getResponse() const;

		class HttpErrorException : public std::exception
		{
			private:

				int _statusCode;

			public:
			
				HttpErrorException(int code) :
					_statusCode(code)
				{}

				int getStatusCode() const
				{
					return (_statusCode);
				}

				const char* what() const throw()
				{
					return ("HTTP Error Exception");
				}
			};

};

#endif





# include "../../include/http/HttpRequest.hpp"
# include "../../include/http/HttpResponse.hpp"
# include "../../include/http/Route.hpp"

ResponseBuilder::ResponseBuilder() :
	_httpResponse()
{}

ResponseBuilder::ReponseBuilder(const HttpRequest& request, const Route& route)
{
	_httpResponse = buildResponse(request, route);
}

ResponseBuilder(const ResponseBuilder & other) :
	_httpResponse(other._httpResponse)
{}

ResponseBuilder & operator=(const ResponseBuilder & other)
{
	if (&other != this) {
		this->_httpResponse = other._httpResponse;
	}
	return (*this);
}

~ResponseBuilder()
{}











/*

Pour construire ta réponse HTTP dans Webserv, tu dois combiner **les informations fournies par la requête (`HttpRequest`)** avec **les règles définies par la configuration (`Route`)**.

Voici un tableau **complet et structuré** des éléments à récupérer dans chaque objet :

---

### 🔵 À récupérer dans `HttpRequest` (la requête client)

| Élément                    | Utilisation                                                      |
| -------------------------- | ---------------------------------------------------------------- |
| `request.getMethod()`      | Vérifier si la méthode est autorisée (GET, POST, etc.)           |
| `request.getPath()`        | Trouver le fichier demandé (à concaténer avec `route.getRoot()`) |
| `request.getHeaders()`     | Gérer `Host`, `Content-Length`, `Content-Type`, etc.             |
| `request.getBody()`        | Pour POST ou PUT (envoyer au CGI, écrire un fichier…)            |
| `request.getQueryString()` | Pour les CGI ou logs (si `?param=value` dans l’URL)              |
| `request.getVersion()`     | Pour formater correctement la ligne de statut HTTP               |

---

### 🟢 À récupérer dans `Route` (la config serveur pour cette URL)

| Élément                     | Utilisation                                               |
| --------------------------- | --------------------------------------------------------- |
| `route.getAllowedMethods()` | Vérifier si la méthode est autorisée                      |
| `route.getRoot()`           | Chemin de base vers les fichiers (à concaténer avec path) |
| `route.getIndex()`          | Fichier à renvoyer si `path` est un dossier               |
| `route.getAutoindex()`      | Si true, générer un listing HTML du dossier               |
| `route.getCgiPath()`        | Si extension `.py` ou `.php`, exécuter le CGI indiqué     |
| `route.getRedirect()`       | Si non vide, redirection HTTP (3xx)                       |
| `route.getUploadDir()`      | Pour stocker les fichiers uploadés (POST sur formulaire)  |
| `route.getErrorPages()`     | Fichier HTML personnalisé pour les erreurs (404, 500…)    |

---

### 🧠 Exemple concret de traitement

1. **Vérification méthode HTTP** :

```cpp
if (!route.isMethodAllowed(request.getMethod()))
	throw HttpErrorException(405); // Method Not Allowed
```

2. **Construction du chemin absolu** :

```cpp
std::string absPath = route.getRoot() + request.getPath();
```

3. **Redirection ?** :

```cpp
if (!route.getRedirect().empty()) {
	response.setStatus(301);
	response.addHeader("Location", route.getRedirect());
	return response;
}
```

4. **CGI ?** :

```cpp
if (isCgiRequest(request.getPath(), route)) {
	// Exécute CGI avec execve, récupère la sortie
}
```

5. **Génération de la réponse** :

* Ligne de statut : `HTTP/1.1 200 OK`
* Headers : `Content-Type`, `Content-Length`, etc.
* Corps : fichier, sortie CGI, ou message d'erreur

---

### ✅ En résumé

| Objet         | Tu récupères…                          | Pour…                               |
| ------------- | -------------------------------------- | ----------------------------------- |
| `HttpRequest` | Méthode, chemin, headers, corps        | Comprendre ce que le client demande |
| `Route`       | Root, méthodes autorisées, cgi, index… | Appliquer la logique de ton serveur |

---


*/
