## Résolution de noms et gestion réseau

> Ces fonctions servent à **traduire des noms en adresses IP**, configurer les ports, etc.

| Fonction         | Rôle principal                                                               |
| ---------------- | ---------------------------------------------------------------------------- |
| `getaddrinfo`    | Résout un nom d’hôte (ex : `localhost` → IP)                                 |
| `freeaddrinfo`   | Libère la mémoire allouée par `getaddrinfo`                                  |
| `getprotobyname` | Récupère le numéro d’un protocole (ex : "tcp")                               |
| `htons`, `htonl` | Convertit des entiers en format réseau                                       |
| `ntohs`, `ntohl` | Convertit des entiers **depuis** le format réseau                            |
| `gai_strerror`   | Convertit un code d'erreur de `getaddrinfo` en chaîne de caractère explicite |

---
### getaddrinfo()

```cpp
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

int getaddrinfo(const char *node, const char *service,
                const struct addrinfo *hints,
                struct addrinfo **res);
```

- `node` : nom d’hôte ou adresse IP (ex. `"localhost"`, `"127.0.0.1"`)
- `service` : nom de service ou numéro de port (ex. `"http"`, `"8080"`)
- `hints` : contraintes souhaitées (famille, type, protocole…)
- `res` : pointeur vers une liste chaînée de résultats (à libérer avec `freeaddrinfo`)

- **Retour :**
    - `0` en cas de succès
    - Code d’erreur sinon (utilisable avec `gai_strerror()`)

La fonction `getaddrinfo()` résout **nom d’hôte + port/service** en une ou plusieurs **structures prêtes pour `connect()` ou `bind()`**. Elle remplace les anciennes fonctions comme `gethostbyname()`.

#### Cas d’usage typiques

- Résoudre `"localhost"` en `127.0.0.1`
- Préparer une `sockaddr` à utiliser avec `socket()`, `connect()`, `bind()`
- Écrire du code compatible IPv4/IPv6

#### Fonctionnement

`getaddrinfo()` retourne une **liste chaînée** de `struct addrinfo`, chacune contenant :
- des champs de filtrage (famille, type…)
- une `sockaddr` prête à l’emploi
- la taille de cette structure

Le champ `hints` peut être partiellement rempli pour filtrer les résultats (ex. `AF_INET`, `SOCK_STREAM`, etc.).

#### Exemple d'utilisation

```cpp
#include <netdb.h>
#include <string.h>

int main()
{
    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo("localhost", "8080", &hints, &res) == 0) {
        // utiliser res->ai_addr avec socket/connect/bind
        freeaddrinfo(res);
    }
    return 0;
}
```

#### Gestion des erreurs

- Retourne un **code d’erreur positif** (ex. `EAI_NONAME`, `EAI_FAIL`)
- Pas de `errno`, mais utilisez `gai_strerror()` pour afficher le message

#### Dans Webserv

Dans Webserv, vous utiliserez `getaddrinfo()` pour :  
✅ **Résoudre les adresses IP et ports** spécifiés dans le fichier de configuration  
✅ **Créer dynamiquement des sockets compatibles IPv4/IPv6**  
✅ **Préparer les structures nécessaires à `bind()` ou `connect()`**


---
### gai_strerror()

```cpp
#include <netdb.h>

const char *gai_strerror(int errcode);
```

- `errcode` : code d’erreur retourné par `getaddrinfo()`

- **Retour :**
    - Pointeur vers une **chaîne littérale décrivant l’erreur**
    - Ne jamais modifier ni libérer cette chaîne

La fonction `gai_strerror()` convertit un **code d’erreur de `getaddrinfo()`** en une **chaîne explicite** compréhensible par l’utilisateur. Elle est équivalente à `strerror()` pour les erreurs réseau retournées par `getaddrinfo()`.

#### Cas d’usage typiques

- Afficher un message d’erreur explicite après un échec de `getaddrinfo()`
- Compléter un log réseau avec une **description humaine de l’erreur**
- Déboguer des erreurs DNS ou de configuration réseau

#### Fonctionnement

`getaddrinfo()` ne définit pas `errno`, mais retourne une **valeur entière** spécifique.  
Ce code doit être passé à `gai_strerror()` pour obtenir une chaîne descriptive.  
La chaîne retournée est **statique**, donc **non modifiable** et **non libérable**.

#### Exemple d'utilisation

```cpp
#include <netdb.h>
#include <stdio.h>

int main()
{
    struct addrinfo *res;
    int ret = getaddrinfo("invalide.local", "http", NULL, &res);
    if (ret != 0)
        fprintf(stderr, "Erreur getaddrinfo: %s\n", gai_strerror(ret));
    return 0;
}
```

#### Codes d’erreurs fréquents

- `EAI_AGAIN` : échec temporaire, réessayer
- `EAI_NONAME` : hôte ou service inconnu
- `EAI_FAIL` : échec non récupérable
- `EAI_MEMORY` : mémoire insuffisante
- `EAI_FAMILY` : famille d’adresses non supportée

#### Dans Webserv

Dans Webserv, vous utiliserez `gai_strerror()` pour :  
✅ **Afficher les erreurs retournées par `getaddrinfo()` de manière lisible**  
✅ **Logguer les échecs de résolution DNS dans vos fichiers de log**  
✅ **Faciliter le débogage des configurations IP/port invalides**


---
### freeaddrinfo()

```cpp
#include <netdb.h>

void freeaddrinfo(struct addrinfo *res);
```

- `res` : pointeur retourné par `getaddrinfo()`

- **Retour :**
    - Aucun (fonction `void`)

La fonction `freeaddrinfo()` libère la mémoire **allouée dynamiquement** par `getaddrinfo()`. Elle doit toujours être appelée après utilisation pour éviter les **fuites mémoire**.

#### Cas d’usage typiques

- Nettoyer les ressources après avoir utilisé `getaddrinfo()`
- Libérer plusieurs adresses retournées par une résolution DNS
- Assurer un comportement propre dans un serveur multi-clients

#### Fonctionnement

Lorsque `getaddrinfo()` réussit, il retourne une **liste chaînée** de structures `addrinfo`, chacune contenant notamment un pointeur vers une `sockaddr`.

`freeaddrinfo()` libère **l’ensemble** de cette liste, y compris tous les champs internes.

#### Exemple d'utilisation

```cpp
#include <netdb.h>
#include <string.h>

int main()
{
    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo("localhost", "8080", &hints, &res) == 0) {
        // ... utiliser res ...
        freeaddrinfo(res); // nettoyage
    }
    return 0;
}
```

#### Gestion des erreurs

Aucune : la fonction est toujours sans effet si `res == NULL`.

#### Dans Webserv

Dans Webserv, vous utiliserez `freeaddrinfo()` pour :  
✅ **Libérer proprement la mémoire** allouée par `getaddrinfo()`  
✅ **Éviter les fuites mémoire** dans la phase d’initialisation réseau  
✅ **Nettoyer automatiquement les résultats** après configuration de socket


---
### freeaddrinfo()

```cpp
#include <netdb.h>

void freeaddrinfo(struct addrinfo *res);
```

- `res` : pointeur retourné par `getaddrinfo()`

- **Retour :**
    - Aucun (fonction `void`)

La fonction `freeaddrinfo()` libère la mémoire **allouée dynamiquement** par `getaddrinfo()`. Elle doit toujours être appelée après utilisation pour éviter les **fuites mémoire**.

#### Cas d’usage typiques

- Nettoyer les ressources après avoir utilisé `getaddrinfo()`
- Libérer plusieurs adresses retournées par une résolution DNS
- Assurer un comportement propre dans un serveur multi-clients

#### Fonctionnement

Lorsque `getaddrinfo()` réussit, il retourne une **liste chaînée** de structures `addrinfo`, chacune contenant notamment un pointeur vers une `sockaddr`.

`freeaddrinfo()` libère **l’ensemble** de cette liste, y compris tous les champs internes.

#### Exemple d'utilisation

```cpp
#include <netdb.h>
#include <string.h>

int main()
{
    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo("localhost", "8080", &hints, &res) == 0) {
        // ... utiliser res ...
        freeaddrinfo(res); // nettoyage
    }
    return 0;
}
```

#### Gestion des erreurs

Aucune : la fonction est toujours sans effet si `res == NULL`.

#### Dans Webserv

Dans Webserv, vous utiliserez `freeaddrinfo()` pour :  
✅ **Libérer proprement la mémoire** allouée par `getaddrinfo()`  
✅ **Éviter les fuites mémoire** dans la phase d’initialisation réseau  
✅ **Nettoyer automatiquement les résultats** après configuration de socket


---
### getprotobyname()

```cpp
#include <netdb.h>

struct protoent *getprotobyname(const char *name);
```

- `name` : nom du protocole (ex. `"tcp"`, `"udp"`)

- **Retour :**
    - Pointeur vers une structure `protoent`
    - `NULL` en cas d’échec (et `h_errno` peut être défini)

La fonction `getprotobyname()` retourne une structure contenant les informations sur un **protocole réseau** identifié par son nom (ex. `"tcp"` ou `"udp"`). Elle est utilisée avec `socket()` pour fournir le **champ `protocol`**.

#### Cas d’usage typiques

- Obtenir le numéro de protocole pour `"tcp"` ou `"udp"`
- Compléter dynamiquement un appel à `socket()` avec le protocole adapté
- Rendre votre code **plus lisible et portable**

#### Fonctionnement

La structure retournée est :

```cpp
struct protoent {
    char  *p_name;    // nom officiel
    char **p_aliases; // alias
    int    p_proto;   // numéro du protocole (ex. IPPROTO_TCP)
};
```

Ce numéro (`p_proto`) est à utiliser dans le troisième argument de `socket()`.

#### Exemple d'utilisation

```cpp
#include <netdb.h>
#include <sys/socket.h>

int main()
{
    struct protoent *proto = getprotobyname("tcp");
    if (proto)
    {
        int sockfd = socket(AF_INET, SOCK_STREAM, proto->p_proto);
        // utilisation de la socket...
    }
    return 0;
}
```

#### Gestion des erreurs

Retourne `NULL` si le protocole est inconnu.  
Utilisez `h_errno` ou comparez le résultat directement.

#### Dans Webserv

Dans Webserv, vous utiliserez `getprotobyname()` pour :  
✅ **Créer des sockets avec le bon protocole (`IPPROTO_TCP`)** sans le coder en dur  
✅ **Améliorer la lisibilité de votre code réseau**  
✅ **Faciliter la gestion multiplateforme** en utilisant les noms standards (`"tcp"`)


---
### htons(), htonl()

```cpp
#include <arpa/inet.h>

uint16_t htons(uint16_t hostshort);
uint32_t htonl(uint32_t hostlong);
```

- `htons()` : convertit un entier 16 bits (port) du format hôte vers le format **réseau**
- `htonl()` : convertit un entier 32 bits (adresse IP) du format hôte vers le format **réseau**

- **Retour :**
    - La valeur convertie en **ordre des octets réseau** (big-endian)

Ces fonctions sont indispensables pour assurer une **interopérabilité réseau** : elles convertissent les entiers en **ordre des octets standardisé**, attendu par tous les protocoles Internet.

#### Cas d’usage typiques

- Convertir un **numéro de port** (ex. `8080`) avant de le mettre dans `sockaddr_in`
- Convertir une **adresse IP** ou toute autre valeur 32 bits destinée à la communication réseau
- Rendre le code portable sur machines **little-endian** et **big-endian**

#### Fonctionnement

Sur la majorité des architectures modernes (Intel, ARM…), les entiers sont stockés en **little-endian**. Le réseau utilise l’ordre **big-endian**.  
Les conversions `htons()` (host to network short) et `htonl()` (host to network long) assurent cette compatibilité.

- Utiliser `htons()` pour `sin_port`
- Utiliser `htonl()` pour `sin_addr.s_addr` (si non converti par `inet_pton()`)

#### Exemple d'utilisation

```cpp
#include <arpa/inet.h>
#include <netinet/in.h>

int main()
{
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8080); // port en format réseau
    addr.sin_addr.s_addr = htonl(0x7f000001); // 127.0.0.1
    return 0;
}
```

#### Gestion des erreurs

Aucune : ces fonctions sont déterministes, sans effet de bord, sans errno.

#### Dans Webserv

Dans Webserv, vous utiliserez `htons()` et `htonl()` pour :  
✅ **Configurer correctement les ports et adresses IP** dans les structures réseau  
✅ **Assurer la portabilité des communications** entre machines différentes  
✅ **Respecter le standard de transmission en big-endian (réseau)**


---
### ntohs(), ntohl()

```cpp
#include <arpa/inet.h>

uint16_t ntohs(uint16_t netshort);
uint32_t ntohl(uint32_t netlong);
```

- `ntohs()` : convertit un entier 16 bits (port) du format **réseau** vers le format **hôte**
- `ntohl()` : convertit un entier 32 bits (IP ou autres) du format **réseau** vers le format **hôte**

- **Retour :**
    - Valeur convertie en **ordre d’octets machine**

Ces fonctions sont les **inverses exactes** de `htons()` et `htonl()`. Elles permettent de **lire correctement** les données reçues du réseau, qui sont toujours en **big-endian**.

#### Cas d’usage typiques

- Lire le **port source** d’un client après un `accept()`
- Lire une **adresse IP** reçue dans une structure `sockaddr_in`
- Interpréter correctement une **valeur 32 bits** venant du réseau (protocole, en-tête…)

#### Fonctionnement

Les architectures modernes stockent les entiers en **little-endian**, tandis que les protocoles réseau utilisent **big-endian**. Ces fonctions garantissent la **bonne interprétation locale** des entiers reçus.

- Utiliser `ntohs()` pour `sin_port`
- Utiliser `ntohl()` pour `sin_addr.s_addr` (si vous le traitez sous forme d’entier brut)

#### Exemple d'utilisation

```cpp
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>

int main()
{
    struct sockaddr_in addr;
    addr.sin_port = htons(8080); // envoie
    printf("Port lisible : %d\n", ntohs(addr.sin_port)); // lecture
    return 0;
}
```

#### Gestion des erreurs

Aucune : fonctions pures, sans effets de bord, ni `errno`.

#### Dans Webserv

Dans Webserv, vous utiliserez `ntohs()` et `ntohl()` pour :  
✅ **Lire les ports et IP reçus du client** dans un format compréhensible localement  
✅ **Afficher ou analyser les adresses et ports** dans vos logs ou traitements  
✅ **Interpréter correctement les valeurs des structures réseau**


---

## Structures associées aux fonctions système

### `struct addrinfo` (utilisée avec `getaddrinfo()`)

```cpp
#include <netdb.h>

struct addrinfo {
    int              ai_flags;
    int              ai_family;      // AF_INET, AF_INET6
    int              ai_socktype;    // SOCK_STREAM, SOCK_DGRAM
    int              ai_protocol;    // IPPROTO_TCP, etc.
    socklen_t        ai_addrlen;
    struct sockaddr *ai_addr;       // adresse utilisable avec bind/connect
    char            *ai_canonname;
    struct addrinfo *ai_next;       // prochain élément de la liste
};
```

**Utilisation typique dans Webserv :**

- `ai_addr` → à passer à `bind()` ou `connect()`
- `ai_family`, `ai_socktype`, `ai_protocol` → à passer à `socket()`