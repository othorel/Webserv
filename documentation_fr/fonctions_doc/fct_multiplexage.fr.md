## Multiplexage I/O — **Gérer plusieurs connexions à la fois**

> Ces fonctions permettent de **surveiller plusieurs sockets en même temps** pour savoir laquelle est prête à lire ou écrire, sans thread.

| Interface | Fonctions associées                                    |
| --------- | ------------------------------------------------------ |
| `select`  | Méthode portable mais limitée (1024 fd max)            |
| `poll`    | Plus souple que `select`, supporte plus de fd          |
| `epoll`   | Pour Linux : `epoll_create`, `epoll_ctl`, `epoll_wait` |
| `kqueue`  | Pour macOS : `kqueue`, `kevent`                        |


---
### select()

```cpp
#include <sys/select.h>

int select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout);
```

- `nfds` : **valeur maximale + 1** des descripteurs surveillés
- `readfds` : ensemble de descripteurs à surveiller en lecture
- `writefds` : ensemble de descripteurs à surveiller en écriture
- `exceptfds` : ensemble pour les exceptions (souvent `NULL`)
- `timeout` : délai d’attente maximum (ou `NULL` pour attendre indéfiniment)

- **Retour :**
    - Nombre total de descripteurs prêts
    - `0` si le délai expire
    - `-1` en cas d’échec, avec `errno` défini

La fonction `select()` permet de **surveiller plusieurs descripteurs** (fichiers, sockets, pipes…) et d’attendre qu’ils deviennent prêts pour la lecture, l’écriture, ou signalent une erreur. C’est un **multiplexeur I/O**.

#### Cas d’usage typiques

- Attendre une **requête HTTP** sur une ou plusieurs sockets
- Vérifier la **disponibilité de données** à lire ou à écrire sans bloquer
- Implémenter un **serveur mono-threadé gérant plusieurs connexions**

#### Fonctionnement

Avant l’appel, on remplit les `fd_set` avec `FD_ZERO()` et `FD_SET(fd, &set)`.  
Après `select()`, on teste la présence d’un fd avec `FD_ISSET(fd, &set)`.  
Le paramètre `nfds` doit être supérieur au **plus grand fd surveillé**.

#### Exemple d'utilisation

```cpp
#include <sys/select.h>
#include <unistd.h>

int main()
{
    fd_set set;
    FD_ZERO(&set);
    FD_SET(0, &set); // surveille stdin

    struct timeval timeout = {5, 0}; // 5 secondes

    if (select(1, &set, NULL, NULL, &timeout) > 0)
        write(1, "Entrée détectée\n", 17);
    return 0;
}
```

#### Gestion des erreurs

Retourne `-1` et définit `errno` :
- `EBADF` : un descripteur est invalide
- `EINTR` : appel interrompu par un signal
- `EINVAL` : `nfds` incohérent ou mauvais `timeout`

#### Dans Webserv

Dans Webserv, vous utiliserez `select()` pour :  
✅ **Gérer plusieurs sockets clients simultanément** sans multithreading  
✅ **Attendre des événements sur `stdin`, socket CGI, etc.**  
✅ **Mettre en place un serveur simple et portable**


---
### poll()

```cpp
#include <poll.h>

int poll(struct pollfd fds[], nfds_t nfds, int timeout);
```

- `fds` : tableau de structures `pollfd` représentant les descripteurs à surveiller
- `nfds` : nombre d’éléments dans le tableau
- `timeout` : délai en millisecondes (`-1` pour bloquer indéfiniment, `0` pour retour immédiat)

- **Retour :**
    - Nombre de descripteurs prêts
    - `0` si le délai expire
    - `-1` en cas d’échec, avec `errno` défini

La fonction `poll()` permet de **surveiller un ensemble de descripteurs** pour des événements comme lecture, écriture ou erreurs. Elle est une **alternative moderne à `select()`**, plus souple et plus évolutive.

#### Cas d’usage typiques

- Gérer un **serveur HTTP** multi-clients
- Attendre des **données d’un CGI**, d’un fichier ou d’une socket
- Implémenter une **file d’événements** non-bloquante

#### Fonctionnement

Chaque élément du tableau `pollfd` contient :
- `fd` : descripteur surveillé
- `events` : événements à surveiller (`POLLIN`, `POLLOUT`, etc.)
- `revents` : champs mis à jour par `poll()` pour indiquer les événements détectés

Contrairement à `select()`, il n’y a **pas de limite sur la valeur des fds**.

#### Exemple d'utilisation

```cpp
#include <poll.h>
#include <unistd.h>
#include <stdio.h>

int main()
{
    struct pollfd fds[1];
    fds[0].fd = 0; // stdin
    fds[0].events = POLLIN;

    int ret = poll(fds, 1, 5000); // 5 secondes
    if (ret > 0 && (fds[0].revents & POLLIN))
        write(1, "Entrée détectée\n", 17);
    return 0;
}
```

#### Gestion des erreurs

Retourne `-1` et définit `errno` :
- `EFAULT` : pointeur invalide
- `EINTR` : appel interrompu par un signal
- `ENOMEM` : ressources insuffisantes

#### Dans Webserv

Dans Webserv, vous utiliserez `poll()` pour :  
✅ **Gérer un grand nombre de connexions clients** sans limite stricte de fd  
✅ **Détecter rapidement l’activité sur sockets et pipes**  
✅ **Mettre en œuvre une boucle d’événements simple et portable**


---
### epoll()

```cpp
#include <sys/epoll.h>
```

#### Création d’un objet epoll

```cpp
int epoll_create(int size);
```

- `size` : nombre **approximatif** de descripteurs surveillés (ignoré depuis Linux 2.6.8)

- **Retour :**
    - Descripteur epoll (`epfd`) en cas de succès
    - `-1` en cas d’échec, avec `errno` défini

#### Ajouter, modifier ou supprimer un fd

```cpp
int epoll_ctl(int epfd, int op, int fd, struct epoll_event *event);
```

- `epfd` : descripteur epoll créé avec `epoll_create()`
- `op` : opération (`EPOLL_CTL_ADD`, `EPOLL_CTL_MOD`, `EPOLL_CTL_DEL`)
- `fd` : descripteur de fichier à surveiller
- `event` : structure contenant les événements à surveiller (`EPOLLIN`, `EPOLLOUT`, etc.)

#### Attendre un événement

```cpp
int epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout);
```

- `epfd` : descripteur epoll
- `events` : tableau rempli par le noyau avec les événements prêts
- `maxevents` : taille du tableau `events`
- `timeout` : délai en ms (`-1` = infini, `0` = non bloquant)

- **Retour de `epoll_wait()` :**
    - Nombre de fds prêts
    - `0` si le délai expire
    - `-1` en cas d’échec


La famille `epoll` fournit une **interface très performante** pour surveiller des milliers de descripteurs. Elle est **spécifique à Linux**, plus rapide et plus scalable que `select()` ou `poll()`.

#### Cas d’usage typiques

- Implémenter un **serveur web à haute performance**
- Gérer **des milliers de connexions simultanées**
- Réagir efficacement à des événements I/O sans boucles coûteuses

#### Fonctionnement

Chaque descripteur est inscrit dans l’objet `epoll` via `epoll_ctl()`.  
L’appel à `epoll_wait()` bloque jusqu’à ce que **des événements I/O** soient disponibles.  
Les `events` retournés contiennent le champ `data.fd` pour identifier le descripteur concerné.

#### Exemple d'utilisation

```cpp
#include <sys/epoll.h>
#include <unistd.h>

int epfd = epoll_create(10);
struct epoll_event ev;
ev.events = EPOLLIN;
ev.data.fd = 0; // stdin
epoll_ctl(epfd, EPOLL_CTL_ADD, 0, &ev);

struct epoll_event events[10];
int n = epoll_wait(epfd, events, 10, 5000);
if (n > 0 && events[0].data.fd == 0)
    write(1, "Entrée détectée\n", 17);
```

#### Gestion des erreurs

- `epoll_create()` : `EINVAL`, `ENOMEM`
- `epoll_ctl()` : `EBADF`, `EEXIST`, `ENOENT`, `EINVAL`
- `epoll_wait()` : `EINTR`, `EINVAL`, `ENOMEM`

#### Dans Webserv

Dans Webserv, vous utiliserez `epoll()` pour :  
✅ **Surveiller efficacement de très nombreuses sockets** clientes  
✅ **Optimiser les performances de la boucle principale du serveur**  
✅ **Réagir rapidement à l’arrivée ou la fermeture des connexions**


---
### kqueue()

```cpp
#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>

int kqueue(void);
```

- **Retour :**
    - Descripteur de file d’événements (identique à un fd)
    - `-1` en cas d’échec, avec `errno` défini

#### Manipulation des événements : `kevent()`

```cpp
int kevent(int kq, const struct kevent *changelist, int nchanges,
           struct kevent *eventlist, int nevents,
           const struct timespec *timeout);
```

- `kq` : descripteur de file d’événements créé avec `kqueue()`
- `changelist` : tableau d’ajouts/suppressions de surveillance
- `nchanges` : nombre d’éléments à modifier
- `eventlist` : tableau à remplir avec les événements déclenchés
- `nevents` : taille maximale du tableau `eventlist`
- `timeout` : délai d’attente (ou `NULL` pour attendre indéfiniment)

- **Retour :**
    - Nombre d’événements prêts
    - `0` si le délai expire
    - `-1` en cas d’échec

La fonction `kqueue()` permet de **surveiller efficacement des descripteurs** (sockets, fichiers, signaux…) sous BSD/macOS. Elle est similaire à `epoll()` sous Linux, mais **plus flexible** et plus générale.

#### Cas d’usage typiques

- Implémenter un **serveur HTTP scalable** sur macOS ou FreeBSD
- Surveiller **fichiers, sockets, signaux** avec un seul mécanisme
- Réagir à des événements complexes comme **fermeture de socket**

#### Fonctionnement

Chaque entrée dans `kevent()` décrit une action (`EV_ADD`, `EV_DELETE`, `EV_ENABLE`, etc.) sur un **identifiant surveillé** (fd, signal…).  
Les événements retournés dans `eventlist` contiennent :

- `ident` : fd concerné
- `filter` : type d’événement (`EVFILT_READ`, `EVFILT_WRITE`, etc.)
- `flags` : état de l’événement (`EV_EOF`, `EV_ERROR`…)

#### Exemple d'utilisation

```cpp
#include <sys/event.h>
#include <unistd.h>

int kq = kqueue();

struct kevent change;
EV_SET(&change, 0, EVFILT_READ, EV_ADD, 0, 0, NULL); // surveille stdin

struct kevent event;
int n = kevent(kq, &change, 1, &event, 1, NULL);
if (n > 0 && event.filter == EVFILT_READ)
    write(1, "Entrée détectée\n", 17);
```

#### Gestion des erreurs

- `kqueue()` : `ENOMEM`, `EMFILE`
- `kevent()` : `EBADF`, `EINVAL`, `EFAULT`

#### Dans Webserv

Dans Webserv, vous utiliserez `kqueue()` pour :  
✅ **Remplacer `select()`/`poll()` sur macOS ou BSD** avec un mécanisme hautes performances  
✅ **Surveiller plusieurs sockets, pipes et fichiers simultanément**  
✅ **Construire une boucle événementielle moderne et portable**

---

## Structures associées aux fonctions système

### `struct epoll_event` (utilisée avec `epoll_ctl()` / `epoll_wait()`)

```cpp
#include <sys/epoll.h>

typedef union epoll_data {
    void    *ptr;
    int      fd;
    uint32_t u32;
    uint64_t u64;
} epoll_data_t;

struct epoll_event {
    uint32_t     events; // EPOLLIN, EPOLLOUT, etc.
    epoll_data_t data;   // identifiant associé
};
```

**Utilisation typique dans Webserv :**

- `data.fd` → identifiant de la socket concernée par un événement
- `events` → pour détecter si lecture, écriture ou erreur

---

## Macros

### **select()**

|Macro|Description|Utilisation dans Webserv|
|---|---|---|
|`FD_ZERO(set)`|Réinitialise l’ensemble de fd|Avant chaque `select()`|
|`FD_SET(fd,set)`|Ajoute un fd à l’ensemble|Surveiller un fd|
|`FD_ISSET(fd,set)`|Le fd est-il prêt ?|Tester si le fd est actif|

### **poll()**

|Macro|Description|Utilisation dans Webserv|
|---|---|---|
|`POLLIN`|Données à lire|Lire une requête ou sortie CGI|
|`POLLOUT`|Prêt à écrire|Envoyer une réponse|
|`POLLERR`|Erreur sur le fd|Gérer les déconnexions|

### **epoll()**

|Macro|Description|Utilisation dans Webserv|
|---|---|---|
|`EPOLLIN`|Lecture possible|Socket prête à lire une requête|
|`EPOLLOUT`|Écriture possible|Socket prête à envoyer une réponse|
|`EPOLLERR`|Erreur|Déconnecter le client|
|`EPOLL_CTL_ADD`|Ajouter un fd à epoll|Lors d’une nouvelle connexion|
|`EPOLL_CTL_MOD`|Modifier les événements surveillés|Changer lecture → écriture|
|`EPOLL_CTL_DEL`|Retirer un fd|Connexion terminée|

---
