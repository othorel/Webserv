# HTTP/1.1


>**HTTP/1.1** (la version 1.1 du protocole HTTP) est un protocole **texte**, structuré en messages clairs et normalisés, échangés entre un **client** et un **serveur** via une connexion TCP.
>Ces messages sont soit des **requêtes** (envoyées par le client), soit des **réponses** (envoyées par le serveur).
>La RFC 2616 est le document officiel qui définit le protocole HTTP/1.1. Il a été **remplacée en 2014** par plusieurs RFC plus claires (RFC 7230 à 7235), mais dans le contexte du projet Webserv, on reste sur **RFC 2616**, car elle regroupe tout en un seul document.

On distingue :

- **La requête :** c'est la demande que le client fait au serveur.
- **La réponse :** c'est le retour que le serveur fait au client.

## Ce que contient la RFC 2616 :

Elle décrit **tout ce qui concerne HTTP/1.1** :

- Comment construire une **requête HTTP**
- Comment formuler une **réponse HTTP**
- Les **méthodes** (`GET`, `POST`, `HEAD`, etc.)
- Les **codes de statut** (`200 OK`, `404 Not Found`, etc.)
- La structure des **en-têtes HTTP** (`Host`, `Content-Type`, etc.)
- La gestion du **cache**, des **cookies**, de la **compression**, des **connexions persistantes**, etc.
- Les comportements du **client**, du **serveur**, et même des **proxies**

## 1. Requête HTTP/1.1

>Une requête HTTP est composée de **trois parties** :

```
1. Ligne de requête (Request-Line)
2. En-têtes (Headers)
3. Corps (Body) — facultatif
```

Chaque ligne du protocole HTTP (y compris les lignes vides) doit se terminer par les deux caractères ASCII `\r\n` (retour chariot + saut de ligne) afin d’assurer un **retour en début de ligne suivi d’un passage à la ligne suivante**, ce qui garantit la compatibilité entre **Unix/Linux, Windows et anciens systèmes Mac**.
On appelle ces deux caractères accolés un CRLF (*Carriage Return + Line Feed*).

### 1.1 Ligne de requête (obligatoire)

>C'est la ligne principale, qui contient l'action que le client demande au serveur.

Forme générale :

```
<METHOD> <URI> <VERSION>

// Exemple :
GET /index.html HTTP/1.1
```

- **METHOD** : action souhaitée (`GET`, `POST`, `HEAD`, `PUT`, `DELETE`, `OPTIONS`, `TRACE`)
- **URI** : ressource demandée (souvent une URI relative comme `/`, `/file.html`)
- **VERSION** : version du HTTP utilisée (pour HTTP/1.1, toujours `HTTP/1.1`)

Points d’attention :

- **Les éléments doivent être séparés par un seul espace (`SP`)**
- **Aucun en-tête ne doit être présent sur cette ligne**
- La version **doit être `HTTP/1.1`**, pas `HTTP/1.0` ni autre
- La `Request-URI` est généralement **relative** (ex : `/fichier.html`)

Les méthodes sont les actions demandées par le client au serveur. Dans Webser, il faut traiter au moins ces 3 :

- **GET** : permet de **demander au serveur l’envoi d’une ressource** (comme une page HTML, une image ou un fichier).
- **POST** : permet de **transmettre des données au serveur**, souvent pour les traiter ou les stocker (ex : formulaire, envoi de fichier).
- **DELETE** : permet de **demander au serveur la suppression d’une ressource identifiée par l’URI**.

### 1.2 En-têtes HTTP

>Un **en-tête HTTP** est une **ligne clé/valeur** placée après la ligne de requête ou de statut, qui **fournit des informations supplémentaires** sur la requête ou la réponse (comme le type de contenu, la taille, l’hôte ou les préférences du client).

Chaque en-tête HTTP est une **ligne texte** qui suit le format :

```
Nom-De-L-En-Tête: valeur
```

- Le caractère `:` **sépare** le nom de l’en-tête et sa valeur; il ne doit pas y avoir d'espace entre le nom et les deux points `:`, mais il peut y avoir des espaces ou tabulations après (généralement un espace), c'est à dire avant la valeur
- Les **noms d’en-têtes ne sont pas sensibles à la casse** : `Content-Type`, `content-type` ou `CONTENT-TYPE` sont tous valides.
- Chaque en-tête se termine par **`CRLF` (`\r\n`)**, comme toutes les lignes du protocole HTTP.
- L’ensemble des en-têtes est suivi d’une **ligne vide** (juste `\r\n`) qui marque la fin de la section des en-têtes.
- Les noms d’en-têtes sont généralement écrits en **Camel-Case** pour la lisibilité (ex : `Content-Length`, `Server`).

Exemples :

```
Host: localhost
User-Agent: Mozilla/5.0
Accept: text/html
```

**En-tête obligatoire** :

- `Host:` → requis par HTTP/1.1 (identifie le serveur demandé)

**En-têtes fréquents** mais optionnels:

- `User-Agent:` → nom du client HTTP (navigateur, script…)
- `Accept:` → formats acceptés (`text/html`, `application/json`, etc.)
- `Content-Type:` → type des données envoyées (utile pour `POST`)
- `Content-Length:` → taille du corps envoyé

### 1.3 Ligne vide

>Une ligne vide (`\r\n`) est **obligatoire** pour séparer les en-têtes du corps.

(Pour la taper la main : appuyer deux fois sur Entrée.)

### 1.4 Corps (facultatif)

>Le **corps HTTP** (ou _body_) est la **partie optionnelle** d’une requête ou d’une réponse qui contient les **données réelles échangées**, comme le contenu d’une page HTML, un fichier, un JSON ou les informations d’un formulaire.

Le corps est présent **uniquement** si la méthode le permet (`POST`, `PUT`, etc.)

## 2. Réponse HTTP/1.1

Une réponse HTTP est composée de :

```
1. Ligne de statut (Status-Line)
2. En-têtes (Headers)
3. Corps (Body) — facultatif
```

Exemple :

```
HTTP/1.1 200 OK                                   // ligne de statut
Content-Type: text/html                           // header
Content-Length: 46                                // header
Connection: close                                 // header
Date: Mon, 27 May 2024 17:30:00 GMT               // header
Server: Webserv/1.0                               // header

<html><body><h1>Bienvenue !</h1></body></html>   // corps
```

### 2.1 Ligne de statut (obligatoire)

>La **ligne de statut** est la **première ligne d’une réponse HTTP** ; elle indique la **version du protocole**, un **code d’état** (comme 200 ou 404), et une **phrase descriptive** résumant le résultat du traitement de la requête.

Forme générale :

```
<VERSION> <CODE> <PHRASE>

// Exemple :
HTTP/1.1 200 OK
```

- **VERSION** : `HTTP/1.1`
- **CODE** : code numérique (`200`, `404`, `500`, etc.)
- **PHRASE** : message court correspondant au code

Points d'attention :

- Chaque **élément est séparé par un seul espace**
- Aucun caractère de contrôle ne doit apparaître dans `Reason-Phrase`
- La ligne **doit se terminer par `\r\n`** comme toute ligne HTTP
- Il faut **respecter la correspondance officielle** entre code et phrase.

Code d'état (ou code de statut) :

Le **code d’état HTTP** est un **nombre à trois chiffres**, compris entre **100 et 599**, qui représente le **résultat du traitement de la requête par le serveur**, toujours accompagné d’une **phrase descriptive standardisée** (appelée _reason-phrase_), comme `200 OK` ou `404 Not Found`.

Codes de statut HTTP  à gérer dans Webserv :

- **1xx — Informations** _(aucun à implémenter dans Webserv): Ces codes (comme `100 Continue`) sont rarement utilisés, et non nécessaires dans Webserv.

- **2xx — Succès**

| Code | Phrase     | Quand l’utiliser                               |
| ---- | ---------- | ---------------------------------------------- |
| 200  | OK         | Requête réussie avec corps renvoyé             |
| 201  | Created    | Fichier ou ressource créée (ex : upload POST)  |
| 204  | No Content | Réussite sans corps (ex : DELETE sans réponse) |

- **3xx — Redirection** : ⚠️ Si le serveur redirige, il doit envoyer un en-tête `Location:`

|Code|Phrase|Quand l’utiliser|
|---|---|---|
|301|Moved Permanently|Redirection permanente (ex : `/dir` → `/dir/`)|
|302|Found|Redirection temporaire|
|303|See Other|POST avec redirection vers une autre URI|
|307|Temporary Redirect|Redirection HTTP/1.1 (rare, mais possible)|

- **4xx — Erreurs côté client**

|Code|Phrase|Quand l’utiliser|
|---|---|---|
|400|Bad Request|Requête mal formée (syntaxiquement incorrecte)|
|403|Forbidden|Accès refusé (ex : fichier non lisible)|
|404|Not Found|Fichier ou URI inexistante|
|405|Method Not Allowed|Méthode HTTP non autorisée (ex : PUT non géré)|
|413|Payload Too Large|Corps de requête trop gros (limite dépassée)|
|414|Request-URI Too Long|URI trop longue|
|415|Unsupported Media Type|Type de contenu non géré|
|431|Request Header Fields Too Large|En-têtes trop gros ou trop nombreux|

⚠️ Pour `405`, il faut ajouter l'en-tête `Allow: GET, POST, DELETE` (selon ce que le serveur accepte).

 - **5xx — Erreurs serveur**

|Code|Phrase|Quand l’utiliser|
|---|---|---|
|500|Internal Server Error|Erreur inattendue côté serveur (ex : CGI planté)|
|501|Not Implemented|Méthode non supportée (`PUT`, `OPTIONS`, etc.)|
|502|Bad Gateway|Erreur CGI / proxy (CGI invalide)|
|503|Service Unavailable|Serveur temporairement indisponible|
|504|Gateway Timeout|CGI ne répond pas à temps|

### 2.2 En-têtes HTTP

>Un **en-tête (*header*) HTTP** est une **ligne clé/valeur** placée après la ligne de requête ou de statut, qui **fournit des informations supplémentaires** sur la requête ou la réponse (comme le type de contenu, la taille, l’hôte ou les préférences du client).

Même format que pour les requêtes, par exemple :

```
Content-Type: text/html
Content-Length: 153
Server: nginx/1.27.5
```

 - En-têtes **obligatoires dans toutes les réponses HTTP/1.1**

| En-tête          | Rôle                                                                                                         |
| ---------------- | ------------------------------------------------------------------------------------------------------------ |
| `Content-Length` | Donne la **taille du corps** (en octets)                                                                     |
| `Content-Type`   | Indique le **type MIME** du contenu renvoyé                                                                  |
| `Connection`     | Permet de spécifier si la connexion doit être **close** ou `keep-alive` (Webserv mettra `Connection: close`) |
Ces en-têtes doivent être présents **dans toutes les réponses** contenant un corps. Pour les réponses **sans corps** (`204`, `304`, ou réponse à `HEAD`), `Content-Length` est soit `0`, soit omis.

- En-têtes **recommandés ou conditionnels** (selon le contexte)

| En-tête             | Quand l’envoyer                                      | Remarques                                                                    |
| ------------------- | ---------------------------------------------------- | ---------------------------------------------------------------------------- |
| `Date`              | Dans toute réponse HTTP                              | Normalement obligatoire en HTTP/1.1, mais dans Webserv, c’est **recommandé** |
| `Server`            | Dans toute réponse                                   | Identifie le serveur (ex : `Webserv/1.0`)                                    |
| `Location`          | Pour les **redirections** `3xx` ou `201`             | Indique la nouvelle URL                                                      |
| `Allow`             | Pour `405 Method Not Allowed`                        | Liste des méthodes autorisées (ex : `Allow: GET, POST`)                      |
| `Content-Location`  | Si la ressource est servie depuis une URI différente | Rare, utile dans les `PUT`, `POST`, `GET` avec redirection logique           |
| `Transfer-Encoding` | Si tu utilises **chunked encoding** (bonus)          | Permet d’envoyer sans `Content-Length`                                       |

- En-têtes **interdits ou inutiles dans Webserv**

|En-tête|Raison|
|---|---|
|`Set-Cookie`|Gère les cookies — hors périmètre|
|`ETag`, `Cache-Control`, `Vary`|Gestion du cache — non demandée dans le sujet|
|`Trailer`, `Upgrade`, `Via`|Réservés à des usages avancés ou HTTP proxy|

### 2.3 Ligne vide

>Une ligne vide (`\r\n`) est **obligatoire** pour séparer les en-têtes du corps.

(Pour la taper la main : appuyer deux fois sur Entrée.)

### 2.4 Corps de la réponse

>Le **corps** de la réponse HTTP est la **partie contenant les données renvoyées par le serveur**, comme une page HTML, une image, un fichier, ou un message JSON, selon ce que la requête demandait.

Contenu retourné au client : HTML, JSON, image, fichier…
**Présent pour les réponses 200, 404, etc.**,  
**Absent pour** : `204 No Content`, `304 Not Modified`, ou une requête `HEAD`.

## Résumé

Requête HTTP/1.1 (envoyée par le client) :

```
<Method> <URI> <Version>\r\n        ← Ligne de requête
Header1: value\r\n                  ← En-têtes (au moins Host:)
Header2: value\r\n
...\r\n
\r\n                                ← Ligne vide obligatoire
<Corps de la requête>              ← Facultatif (ex : POST, PUT)
```

- **Ligne vide** obligatoire avant le corps  
- **Pas de ligne vide après le corps**

Réponse HTTP/1.1 (envoyée par le serveur) :

```
<Version> <Code> <Reason-Phrase>\r\n    ← Ligne de statut
Header1: value\r\n                      ← En-têtes (Content-Length, etc.)
Header2: value\r\n
...\r\n
\r\n                                    ← Ligne vide obligatoire
<Corps de la réponse>                  ← Contenu (HTML, image, JSON…)
```

- **Ligne vide** obligatoire avant le corps  
- **Pas de ligne vide après le corps**

>Contrairement aux en-têtes, **aucune ligne vide n’est nécessaire après le corps**, car **le client et le serveur savent exactement quand le corps se termine** grâce à l’en-tête `Content-Length` (ou `Transfer-Encoding: chunked`, dans les cas avancés).
>Ainsi, **la ligne vide obligatoire ne sert qu’à séparer les en-têtes du corps**, jamais à marquer la fin de la requête ou de la réponse.
