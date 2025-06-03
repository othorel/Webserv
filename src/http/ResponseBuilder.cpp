#include <string>
#include <stdexcept>
#include <iostream>
#include "../../include/http/ResponseBuilder.hpp"
#include "../../include/http/HttpRequest.hpp"
#include "../../include/http/HttpResponse.hpp"
#include "../../include/config/Location.hpp"

ResponseBuilder::ResponseBuilder() :
	_httpResponse()
{}

ResponseBuilder::ResponseBuilder(
		const HttpRequest& request,
		const std::map<std::string, Location> & routes)
{
	_httpResponse = buildResponse(request, routes);
}

ResponseBuilder::ResponseBuilder(const ResponseBuilder & other) :
	_httpResponse(other._httpResponse)
{}

ResponseBuilder & ResponseBuilder::operator=(const ResponseBuilder & other)
{
	if (&other != this) {
		this->_httpResponse = other._httpResponse;
	}
	return (*this);
}

ResponseBuilder::~ResponseBuilder()
{}

const Location & ResponseBuilder::findMatchingRoute(
		const std::map<std::string, Location> & routes,
		const std::string & target) const
{
	std::string bestMatch = "";
	std::map<std::string, Location>::const_iterator it;
	for (it = routes.begin(); it != routes.end(); ++it)
	{
		const std::string& path = it->first;
		if (target.compare(0, path.size(), path) == 0)
		{
			if (path.size() > bestMatch.size())
				bestMatch = path;
		}
	}
	if (bestMatch.empty())
		throw std::runtime_error("No matching route found");
	return (routes.find(bestMatch)->second);
}

const HttpResponse & ResponseBuilder::buildResponse(
		const HttpRequest& request,
		const std::map<std::string, Location> & routes)
{
	// 	1. Trouver la Location correspondant à l’URI
	// Extraire request.getTarget() (ex: /images/cat.jpg)
	// Chercher la Location la plus spécifique (par exemple /images plutôt que /)+
	try {
		const Location route = findMatchingRoute(routes, request.getTarget());
	}
	catch (const std::runtime_error & e) {
		std::cerr << "Erreur attrapée : " << e.what() << std::endl;
		_httpResponse = buildError(404);
		return (_httpResponse);
	}



// 2. Vérifier que la méthode HTTP est autorisée
// Vérifier que request.getMethod() est dans location.getAllowedMethods()

// cpp
// Copier
// Modifier
// if (!location.isAllowedMethod(request.getMethod()))
//     return buildError(405); // Method Not Allowed
// 3. Calculer le chemin du fichier à servir
// Combiner la racine (location.getRoot()) avec le chemin de la requête :

// cpp
// Copier
// Modifier
// std::string fullPath = location.getRoot() + request.getTarget().substr(location.getPath().length());
// 4. Gérer les cas particuliers
// a. Redirection
// Si ta Location définit une redirection (return 301 http://...), construis directement une réponse avec :

// cpp
// Copier
// Modifier
// statusCode = 301;
// headers["Location"] = "http://...";
// b. Autoindex
// Si le chemin pointe vers un dossier, et que le fichier index.html est absent :

// Si autoindex == true, génère une page HTML listant les fichiers

// Sinon, retourne 403 Forbidden ou 404 Not Found

// 5. Gérer le CGI (si activé dans la location)
// Si location.getCgiPath() est non vide et que le fichier est un CGI (ex: .py, .php) :

// fork()

// execve() du script

// Récupérer la sortie via pipe

// En faire le corps de la HttpResponse

// 6. Lire le fichier demandé
// Ouvrir le fichier en lecture binaire

// Lire son contenu

// Déduire le type MIME depuis l’extension (ex: .html, .jpg, etc.)

// Remplir :

// _statusCode = 200

// _headers["Content-Type"] = "text/html" (ou autre)

// _headers["Content-Length"] = taille du corps

// _body = contenu du fichier

// 7. Créer la réponse HTTP
// Construire l’objet :

// cpp
// Copier
// Modifier
// HttpResponse response("HTTP/1.1", 200, headers, body);
// Appeler response.toRawString() pour envoyer la réponse finale au client











/*

Pour construire ta réponse HTTP dans Webserv, tu dois combiner **les informations fournies par la requête (`HttpRequest`)** avec **les règles définies par la configuration (`Location`)**.

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

### 🟢 À récupérer dans `Location` (la config serveur pour cette URL)

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
| `Location`       | Root, méthodes autorisées, cgi, index… | Appliquer la logique de ton serveur |

---


*/
