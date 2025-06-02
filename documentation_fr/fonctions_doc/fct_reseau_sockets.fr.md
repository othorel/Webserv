## Réseau — **Sockets**

> Ce sont les fonctions centrales pour créer le **serveur web TCP**, accepter des connexions, envoyer/recevoir des requêtes.

| Fonction      | Rôle principal                                                                      |
| ------------- | ----------------------------------------------------------------------------------- |
| `socket`      | Crée une socket TCP                                                                 |
| `bind`        | Associe une socket à une IP et un port                                              |
| `listen`      | Met la socket en mode écoute                                                        |
| `accept`      | Accepte une connexion entrante                                                      |
| `connect`     | Tente de se connecter à une socket distante                                         |
| `recv`        | Reçoit des données                                                                  |
| `send`        | Envoie des données                                                                  |
| `setsockopt`  | Modifie des options sur une socket                                                  |
| `getsockname` | Récupère l’adresse locale de la socket                                              |
| `socketpair`  | Crée une paire de sockets connectées (ex : pipe bidirectionnel entre processus CGI) |

---
### socket()

```cpp
#include <sys/types.h>
#include <sys/socket.h>

int socket(int domain, int type, int protocol);
```

- `domain` : famille d’adresses (ex. `AF_INET`, `AF_UNIX`)
- `type` : type de socket (ex. `SOCK_STREAM`, `SOCK_DGRAM`)
- `protocol` : protocole spécifique (0 pour le protocole par défaut)

- **Retour :**
    - Descripteur de socket en cas de succès
    - `-1` en cas d’échec, avec `errno` défini

La fonction `socket()` crée un **point de communication réseau**. Elle initialise une socket sans la connecter ni l'associer à un port. C’est la **première étape** de toute communication via TCP ou UDP.

#### Cas d’usage typiques

- Créer une socket serveur pour recevoir des connexions TCP
- Créer une socket cliente pour initier une connexion vers un serveur
- Définir des sockets locales entre processus (`AF_UNIX`)

#### Fonctionnement

La socket créée est un **descripteur de fichier**, que l’on peut utiliser avec `read()`, `write()`, `recv()`, `send()`, etc.  
Elle n’est pas encore connectée ni liée à une adresse : d'autres appels (`bind()`, `connect()`) sont nécessaires selon les cas.

#### Exemple d'utilisation

```cpp
#include <sys/socket.h>
#include <netinet/in.h>

int main()
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
        return 1;
    return 0;
}
```

#### Gestion des erreurs

Retourne `-1` et définit `errno` :
- `EAFNOSUPPORT` : famille d’adresses non supportée
- `EPROTONOSUPPORT` : protocole non supporté
- `EMFILE` ou `ENFILE` : trop de fichiers ouverts
- `ENOBUFS` ou `ENOMEM` : ressources insuffisantes

#### Dans Webserv

Dans Webserv, vous utiliserez `socket()` pour :  
✅ **Créer une socket serveur TCP/IP** pour écouter sur un port  
✅ **Créer des sockets pour les connexions client** entrantes ou sortantes  
✅ **Initialiser les communications réseau** en choisissant famille, type et protocole


---
### bind()

```cpp
#include <sys/types.h>
#include <sys/socket.h>

int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
```

- `sockfd` : descripteur de socket à associer
- `addr` : pointeur vers une structure d’adresse (ex. `struct sockaddr_in`)
- `addrlen` : taille de la structure pointée

- **Retour :**
    - `0` en cas de succès
    - `-1` en cas d’échec, avec `errno` défini

La fonction `bind()` associe une **socket non connectée** à une **adresse locale** (IP + port). Elle est indispensable côté serveur pour écouter les connexions entrantes sur un port donné.

#### Cas d’usage typiques

- Attacher une socket serveur à une adresse IP et un port précis
- Réserver un port local avant de recevoir des connexions
- Associer une socket Unix à un chemin de fichier (`AF_UNIX`)

#### Fonctionnement

L’adresse fournie (ex. `sockaddr_in`) doit être correctement remplie avec :

- `sin_family` (souvent `AF_INET`)
- `sin_port` (avec `htons()`)
- `sin_addr` (avec `INADDR_ANY` ou une adresse IP)

Après un `bind()`, la socket est liée à cette adresse, et peut alors être utilisée avec `listen()` ou `recvfrom()`.

#### Exemple d'utilisation

```cpp
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>

int main()
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr;

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8080);
    addr.sin_addr.s_addr = INADDR_ANY;

    bind(sockfd, (struct sockaddr*)&addr, sizeof(addr));
    return 0;
}
```

#### Gestion des erreurs

Retourne `-1` et définit `errno` :
- `EADDRINUSE` : port déjà utilisé
- `EBADF` : socket invalide
- `EACCES` : permission refusée (port réservé)
- `EINVAL` : socket déjà liée, ou paramètres incohérents

#### Dans Webserv

Dans Webserv, vous utiliserez `bind()` pour :  
✅ **Associer la socket à une IP et un port définis dans le fichier de configuration**  
✅ **Réserver le port d'écoute du serveur HTTP**  
✅ **Préparer la socket pour qu’elle accepte des connexions entrantes via `listen()`**


---
### listen()

```cpp
#include <sys/socket.h>

int listen(int sockfd, int backlog);
```

- `sockfd` : descripteur de socket déjà lié avec `bind()`
- `backlog` : nombre maximal de connexions en file d’attente

- **Retour :**
    - `0` en cas de succès
    - `-1` en cas d’échec, avec `errno` défini

La fonction `listen()` transforme une socket en **socket passive**, capable d’**accepter des connexions entrantes**. Elle est utilisée sur une socket de type `SOCK_STREAM` après `bind()`.

#### Cas d’usage typiques

- Activer une socket serveur TCP pour recevoir des connexions
- Gérer un grand nombre de clients simultanément
- Initialiser l’écoute d’un port réseau

#### Fonctionnement

Une fois `listen()` appelée, le système commence à **mettre en file d’attente les demandes de connexion** entrantes sur la socket. Le paramètre `backlog` détermine le **nombre maximal** de connexions en attente non acceptées.

Il ne démarre pas la communication — il prépare la socket pour que `accept()` puisse ensuite établir les connexions.

#### Exemple d'utilisation

```cpp
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>

int main()
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr;

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8080);
    addr.sin_addr.s_addr = INADDR_ANY;

    bind(sockfd, (struct sockaddr*)&addr, sizeof(addr));
    listen(sockfd, 10); // 10 connexions en attente max
    return 0;
}
```

#### Gestion des erreurs

Retourne `-1` et définit `errno` :
- `EADDRINUSE` : port déjà utilisé
- `EINVAL` : socket non valide ou déjà en écoute
- `ENOTSOCK` : `sockfd` n’est pas une socket
- `EOPNOTSUPP` : type de socket non supporté pour `listen()`

#### Dans Webserv

Dans Webserv, vous utiliserez `listen()` pour :  
✅ **Mettre la socket en état d’écoute** après l’avoir liée avec `bind()`  
✅ **Accepter les connexions TCP entrantes des clients**  
✅ **Gérer le nombre de connexions en attente** via le paramètre `backlog`


---
### accept()

```cpp
#include <sys/socket.h>

int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
```

- `sockfd` : descripteur de socket en écoute (`listen()`)
- `addr` : pointeur vers une structure recevant l’adresse du client
- `addrlen` : pointeur vers la taille de la structure (modifiée à la sortie)

- **Retour :**
    - Nouveau descripteur de socket connecté (client)
    - `-1` en cas d’échec, avec `errno` défini

La fonction `accept()` extrait la **première connexion en attente** de la file d’attente de la socket, et retourne un **nouveau descripteur** de socket connecté au client.

#### Cas d’usage typiques

- Créer une connexion avec un client HTTP
- Gérer chaque client dans un processus ou un thread distinct
- Accepter une ou plusieurs connexions entrantes sur le serveur

#### Fonctionnement

`accept()` est **bloquante** par défaut : elle attend jusqu’à ce qu’un client se connecte.  
La nouvelle socket retournée est dédiée à la communication avec ce **client unique**.  
Le descripteur d’origine (`sockfd`) continue à écouter d’autres connexions.

Les paramètres `addr` et `addrlen` permettent de récupérer l’adresse du client.  
Ils peuvent être mis à `NULL` si cette information n’est pas requise.

#### Exemple d'utilisation

```cpp
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>

int main()
{
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in serv_addr;

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(8080);
    serv_addr.sin_addr.s_addr = INADDR_ANY;

    bind(server_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    listen(server_fd, 5);

    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);

    int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &addr_len);
    return 0;
}
```

#### Gestion des erreurs

Retourne `-1` et définit `errno` :
- `EAGAIN` / `EWOULDBLOCK` : socket non bloquante, aucune connexion en attente
- `EBADF` : descripteur invalide
- `ECONNABORTED` : connexion interrompue avant acceptation
- `EINTR` : appel interrompu par un signal
- `EMFILE` / `ENFILE` : trop de fichiers ouverts

#### Dans Webserv

Dans Webserv, vous utiliserez `accept()` pour :  
✅ **Accepter les connexions entrantes** d’un client HTTP sur la socket serveur  
✅ **Créer un nouveau socket client dédié** pour dialoguer avec un visiteur  
✅ **Extraire l’adresse du client** (utile pour les logs ou restrictions)


---
### connect()

```cpp
#include <sys/socket.h>

int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
```

- `sockfd` : descripteur de socket non connectée (généralement créée avec `socket()`)
- `addr` : adresse du serveur à joindre (ex. `sockaddr_in`)
- `addrlen` : taille de la structure pointée

- **Retour :**
    - `0` en cas de succès
    - `-1` en cas d’échec, avec `errno` défini

La fonction `connect()` permet à une **socket cliente** d’initier une **connexion vers une adresse distante**. Elle est utilisée côté client dans une communication réseau (ex. HTTP, TCP, etc.).

#### Cas d’usage typiques

- Se connecter à un **serveur distant** (ex. `localhost:8080`)
- Établir une liaison TCP sortante pour envoyer une requête
- Créer une connexion vers un **CGI local** via socket Unix (`AF_UNIX`)

#### Fonctionnement

`connect()` établit une connexion réseau entre la socket et l’adresse indiquée.  
Sur les sockets **bloquantes** (par défaut), l’appel attend jusqu’à l’établissement ou l’échec.  
Sur les sockets non bloquantes (`O_NONBLOCK`), l’appel peut retourner immédiatement, et la connexion est suivie via `select()` ou `poll()`.

#### Exemple d'utilisation

```cpp
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

int main()
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);

    connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr));
    return 0;
}
```

#### Gestion des erreurs

Retourne `-1` et définit `errno` :
- `ECONNREFUSED` : le serveur refuse la connexion
- `ETIMEDOUT` : délai de connexion dépassé
- `EHOSTUNREACH` / `ENETUNREACH` : hôte ou réseau inaccessible
- `EINPROGRESS` : en cours (pour socket non bloquante)
- `EISCONN` : socket déjà connectée

#### Dans Webserv

Dans Webserv, vous utiliserez `connect()` pour :  
✅ **Initier une connexion sortante** (ex. vers un serveur CGI via socket locale)  
✅ **Établir une communication réseau côté client** si le serveur doit agir en tant que client  
✅ **Vérifier la disponibilité d’un service cible** lors d’interactions indirectes


---
### recv()

```cpp
#include <sys/socket.h>

ssize_t recv(int sockfd, void *buf, size_t len, int flags);
```

- `sockfd` : descripteur de socket connectée
- `buf` : pointeur vers un tampon de réception
- `len` : taille maximale à lire en octets
- `flags` : options supplémentaires (souvent `0`)

- **Retour :**
    - Nombre d’octets reçus
    - `0` si la connexion est **fermée proprement** par l’autre côté
    - `-1` en cas d’échec, avec `errno` défini

La fonction `recv()` lit des **données depuis une socket connectée**, typiquement une socket TCP. Elle est similaire à `read()`, mais avec des options spécifiques aux sockets.

#### Cas d’usage typiques

- Lire la **requête HTTP** envoyée par un client
- Recevoir les **données de réponse d’un CGI**
- Implémenter un protocole de lecture non-bloquant avec `MSG_DONTWAIT`

#### Fonctionnement

`recv()` copie au plus `len` octets depuis la socket dans `buf`.  
Si aucune donnée n’est disponible :

- Sur une socket **bloquante** : elle attend.
- Sur une socket **non bloquante** : elle retourne `-1` avec `errno == EAGAIN`.

Des `flags` comme `MSG_PEEK`, `MSG_WAITALL`, ou `MSG_DONTWAIT` permettent d’ajuster le comportement.

#### Exemple d'utilisation

```cpp
#include <sys/socket.h>
#include <unistd.h>
#include <stdio.h>

int main()
{
    char buffer[1024];
    int client_fd = /* socket client déjà connectée */;
    ssize_t n = recv(client_fd, buffer, sizeof(buffer), 0);
    if (n > 0)
        write(1, buffer, n); // afficher ce qui a été reçu
    return 0;
}
```

#### Gestion des erreurs

Retourne `-1` et définit `errno` :

- `EAGAIN` / `EWOULDBLOCK` : aucune donnée disponible (socket non bloquante)
- `ECONNRESET` : connexion réinitialisée par le client
- `ENOTCONN` : socket non connectée
- `EINTR` : appel interrompu par un signal

#### Dans Webserv

Dans Webserv, vous utiliserez `recv()` pour :  
✅ **Lire la requête HTTP** envoyée par un client sur une socket connectée  
✅ **Recevoir la sortie d’un CGI** via un pipe ou socket locale  
✅ **Gérer des lectures conditionnelles** avec timeout ou non-bloquantes


---
### send()

```cpp
#include <sys/socket.h>

ssize_t send(int sockfd, const void *buf, size_t len, int flags);
```

- `sockfd` : descripteur de socket connectée
- `buf` : pointeur vers les données à envoyer
- `len` : taille des données à envoyer (en octets)
- `flags` : options supplémentaires (souvent `0`)

- **Retour :**
    - Nombre d’octets envoyés
    - `-1` en cas d’échec, avec `errno` défini

La fonction `send()` permet d’**envoyer des données via une socket connectée**. Elle est similaire à `write()`, mais offre des **options spécifiques aux communications réseau**.

#### Cas d’usage typiques

- Envoyer une **réponse HTTP** à un client
- Transmettre des **données à un CGI** via une socket locale
- Gérer l’envoi en **mode non-bloquant** ou partiel

#### Fonctionnement

`send()` écrit jusqu’à `len` octets du tampon `buf` dans la socket.  
Elle n’assure pas forcément que **toutes les données sont envoyées** : en cas de buffer plein ou réseau saturé, l’appel peut retourner avant.

Des `flags` comme `MSG_NOSIGNAL` (évite `SIGPIPE` si la socket est fermée) peuvent être utiles pour renforcer la robustesse.

#### Exemple d'utilisation

```cpp
#include <sys/socket.h>
#include <string.h>

int main()
{
    int client_fd = /* socket connectée au client */;
    const char *msg = "HTTP/1.1 200 OK\r\n\r\nBonjour";
    send(client_fd, msg, strlen(msg), 0);
    return 0;
}
```

#### Gestion des erreurs

Retourne `-1` et définit `errno` :
- `EPIPE` : la socket a été fermée par l’autre côté (risque de `SIGPIPE`)
- `EAGAIN` / `EWOULDBLOCK` : socket non bloquante, tampon plein
- `ENOTCONN` : socket non connectée
- `EINTR` : appel interrompu par un signal

#### Dans Webserv

Dans Webserv, vous utiliserez `send()` pour :  
✅ **Envoyer la réponse HTTP** générée par le serveur au client  
✅ **Communiquer avec un CGI** ou un processus enfant via socket ou pipe  
✅ **Contrôler les erreurs d’envoi** (ex. client déconnecté, tampon saturé)


---
### setsockopt()

```cpp
#include <sys/types.h>
#include <sys/socket.h>

int setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen);
```

- `sockfd` : descripteur de socket concernée
- `level` : niveau du protocole (`SOL_SOCKET`, `IPPROTO_TCP`, etc.)
- `optname` : nom de l’option à modifier (ex. `SO_REUSEADDR`)
- `optval` : pointeur vers la valeur de l’option
- `optlen` : taille de la valeur pointée

- **Retour :**
    - `0` en cas de succès
    - `-1` en cas d’échec, avec `errno` défini

La fonction `setsockopt()` permet de **modifier le comportement d’une socket** en ajustant ses options internes, avant ou après sa création. Elle est souvent utilisée pour activer des **options importantes pour un serveur** réseau.

#### Cas d’usage typiques

- Activer `SO_REUSEADDR` pour **réutiliser rapidement un port**
- Activer `SO_KEEPALIVE` pour surveiller les connexions longues
- Définir un **timeout de lecture/écriture** (`SO_RCVTIMEO`, `SO_SNDTIMEO`)

#### Fonctionnement

Chaque option a son propre niveau (`SOL_SOCKET` pour options génériques).  
On passe un pointeur vers la valeur de l’option, ainsi que sa taille.  
Des options sont des booléens (entiers) ; d’autres attendent des structures (ex. `struct timeval`).

Cette fonction peut être utilisée avant ou après le `bind()`.

#### Exemple d'utilisation

```cpp
#include <sys/socket.h>

int main()
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    int optval = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
    return 0;
}
```

#### Gestion des erreurs

Retourne `-1` et définit `errno` :
- `EBADF` : socket invalide
- `ENOPROTOOPT` : option non supportée pour ce niveau
- `EFAULT` : `optval` pointe vers un espace mémoire invalide
- `EINVAL` : valeur ou taille incorrecte

#### Dans Webserv

Dans Webserv, vous utiliserez `setsockopt()` pour :  
✅ **Activer `SO_REUSEADDR`** pour éviter l’erreur "Address already in use"  
✅ **Configurer les timeouts** et comportements spécifiques des sockets  
✅ **Ajuster dynamiquement le comportement réseau** selon le contexte


---
### getsockname()

```cpp
#include <sys/socket.h>

int getsockname(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
```

- `sockfd` : descripteur de socket dont on veut connaître l’adresse locale
- `addr` : pointeur vers une structure qui recevra l’adresse
- `addrlen` : pointeur vers la taille de la structure (modifiée à la sortie)

- **Retour :**
    - `0` en cas de succès
    - `-1` en cas d’échec, avec `errno` défini

La fonction `getsockname()` permet de **récupérer l’adresse locale associée à une socket** (après `bind()` ou `connect()`). Elle est utile lorsque le port ou l’IP a été attribué dynamiquement.

#### Cas d’usage typiques

- Vérifier **le port local utilisé**, si on a demandé `port = 0`
- Obtenir l’**IP d’attachement réel** de la socket
- Récupérer l’adresse Unix liée à une socket `AF_UNIX`

#### Fonctionnement

`getsockname()` remplit la structure `addr` avec l’adresse actuellement assignée à la socket (via `bind()` ou `connect()`), et ajuste la taille réelle dans `addrlen`.  
Elle ne concerne que l’**adresse locale** de la socket, pas celle du pair distant.

#### Exemple d'utilisation

```cpp
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>

int main()
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);

    addr.sin_family = AF_INET;
    addr.sin_port = 0; // demande d’un port libre
    addr.sin_addr.s_addr = INADDR_ANY;

    bind(sockfd, (struct sockaddr*)&addr, sizeof(addr));
    getsockname(sockfd, (struct sockaddr*)&addr, &len);

    printf("Port attribué : %d\n", ntohs(addr.sin_port));
    return 0;
}
```

#### Gestion des erreurs

Retourne `-1` et définit `errno` :
- `EBADF` : socket invalide
- `ENOTSOCK` : `sockfd` n’est pas une socket
- `EFAULT` : pointeur `addr` invalide

#### Dans Webserv

Dans Webserv, vous utiliserez `getsockname()` pour :  
✅ **Récupérer dynamiquement le port utilisé** si configuré sur `0`  
✅ **Obtenir l’adresse IP locale** effective associée à une socket  
✅ **Faire du log ou du debug précis** sur les liaisons en cours


---
### socketpair()

```cpp
#include <sys/socket.h>

int socketpair(int domain, int type, int protocol, int sv[2]);
```

- `domain` : famille de sockets (souvent `AF_UNIX`)
- `type` : type de socket (souvent `SOCK_STREAM`)
- `protocol` : 0 (choix par défaut)
- `sv` : tableau de deux descripteurs de socket connectés entre eux

- **Retour :**
    - `0` en cas de succès
    - `-1` en cas d’échec, avec `errno` défini

La fonction `socketpair()` crée **deux sockets connectées l’une à l’autre**, souvent utilisées pour **communiquer entre processus parent et enfant** après un `fork()`. Contrairement à `pipe()`, la communication est **bidirectionnelle**.

#### Cas d’usage typiques

- Remplacer `pipe()` pour un **échange bidirectionnel**
- Communiquer avec un processus CGI local de manière complète
- Créer un **canal de communication sécurisé** entre deux parties d’un même programme

#### Fonctionnement

Les deux sockets retournées dans `sv[0]` et `sv[1]` sont **équivalentes à deux extrémités connectées** (comme un mini réseau privé local).  
Elles peuvent être utilisées avec `read()`, `write()`, `recv()`, `send()`…  
Fonctionne uniquement avec `AF_UNIX` comme `domain` sur la plupart des systèmes.

#### Exemple d'utilisation

```cpp
#include <sys/socket.h>
#include <unistd.h>

int main()
{
    int sv[2];
    socketpair(AF_UNIX, SOCK_STREAM, 0, sv);

    pid_t pid = fork();

    if (pid == 0) {
        close(sv[0]);
        write(sv[1], "Bonjour parent", 15);
    } else {
        close(sv[1]);
        char buf[100];
        read(sv[0], buf, sizeof(buf));
    }
    return 0;
}
```

#### Gestion des erreurs

Retourne `-1` et définit `errno` :
- `EAFNOSUPPORT` : `domain` non supporté
- `EPROTONOSUPPORT` : protocole non supporté
- `EINVAL` : type ou combinaison invalide
- `EMFILE` / `ENFILE` : trop de fichiers ouverts
- `EFAULT` : pointeur invalide

#### Dans Webserv

Dans Webserv, vous utiliserez `socketpair()` pour :  
✅ **Communiquer avec un processus CGI local** de manière bidirectionnelle  
✅ **Échanger des données de façon sûre** entre deux processus  
✅ **Remplacer `pipe()` si une communication dans les deux sens est requise**

---

## Macros

### **Sockets et réseau (`sys/socket.h`)**

|Macro|Description|Utilisation dans Webserv|
|---|---|---|
|`AF_INET`|Famille d’adresses IPv4|Utilisé avec `socket()`, `bind()`|
|`SOCK_STREAM`|Type TCP|Type principal pour serveur HTTP|
|`IPPROTO_TCP`|Protocole TCP|Complète la socket si nécessaire|
|`SO_REUSEADDR`|Réutilisation du port (évite "Address in use")|Utilisé avec `setsockopt()`|
