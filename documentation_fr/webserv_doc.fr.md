
Cette fiche a pour but de donner une vue d'ensemble du projet Webserv de l'école 42, ainsi que plusieurs informations et précisions utiles à la bonne compréhension du sujet.

Dans le répertoire `/documentation` vous trouverez d'autres fiches qui approfondissent certains sujets :

- `fichiers_de_config.fr.md`-> à propos des fichiers de configuration
- `HTTP1.1.fr.md` -> à propos du protocole HTTP version 1.1 tel qu'il apparaît dans le RFC 2616
- `CGI.fr.md` -> à propos des scripts CGI
- `fonctions_doc/` -> répertoire contenant plusieurs fichiers à expliquant les fonctions autorisées


---

# Un serveur web en C++

## Définition

Un serveur web est un programme qui :

- Écoute des requêtes HTTP provenant de clients (comme des navigateurs)  
- Analyse ces requêtes  
- Recherche la ressource demandée (fichier, script, etc.)  
- Et renvoie une réponse HTTP structurée, généralement du HTML ou un autre type de contenu.

> En résumé, un serveur web c'est un traducteur entre un client qui pose une question, et un système de fichiers ou d'exécution qui fournit une réponse.

## Fichier de configuration

Le programme Webserv doit être lancé avec en argument un chemin vers un fichier de configuration : C’est un **fichier texte**, généralement avec l’extension `.conf`, qui décrit **comment le serveur Web doit se comporter**, et qui devra être parsé à l'éxécution.
Le programme doit **lire ce fichier**, puis **configurer dynamiquement le serveur** en fonction de ce qu’il contient.

>Si le programme est lancé sans argument, il doit alors utiliser un fichier de configuration par défaut, placé à un chemin connu et codé en dur (par exemple : `"./default.conf"` ou `"/etc/webserv.conf"`).

Il n'existe pas de grammaire imposée pour organiser un fichier de configuration, mais pour Webserv, on s'aligne la plupart du temps sur les fichiers de config utilisés par NGINX (c'est un un serveur web léger hautement configurable). Ils sont basés sur :

1. **Des blocs** (ex: `server`, `location`)
2. **Des directives** (ex: `listen`, `root`, `index`) terminées par `;`
3. Une **structure hiérarchique** logique

***-> Les fichiers de cobfiguration sont abordées en détail dans le fichier `fichiers_de_config.fr.md`***

## 1. Ecouter

Le serveur ouvre une **socket** TCP sur un **port** (généralement 80 ou 8080) pour attendre les connexions entrantes.

## 2. Accepter

Le serveur accepte une connexion et reçoit une requête HTP, par exemple:

```
GET /index.html HTTP/1.1
Host: localhost
```
## 3. Interpréter

Le serveur analyse la requête:

- méthode (`GET`, `POST`, etc.)
- chemin (`/index.html`)
- en-têtes (`Host`, `Content-Type`, etc.)

## 4 Répondre

Le serveur construit une réponse HTTP avec:

- une **ligne de statut** (`HTTP/1.1 200 OK`)
- des **en-têtes** (`Content-Type`, `Content-Length`, etc.)
- un **corps** (souvent du HTML, JSON, ou une image)

## 5. Gérer plusieurs clients

Il doit pouvoir gérer **plusieurs connexions en parallèle**, sans bloquer.

## 6. Lancer des scripts (CGI)

Si la requête cible un script (ex : `.py`, `.php`), il doit :

- créer un processus avec `fork()`
- exécuter le script avec `execve()`
- récupérer sa sortie
- l’envoyer comme réponse

## Webserv en résumé

En résumé, le programme Webserv va devoir:

- lire et appliquer un fichier de configuration
 - écouter des sockets
 - comprendre le protocole HTTP (requêtes / réponses)
 - ouvrir et lire des fichiers
 - exécuter des scripts (CGI)
 - gérer des erreurs (404, 500, etc.)
 - pouvoir configurer plusieurs serveurs


---

# Quelques définitions de base

## Client

> Un **client** est un programme (souvent un **navigateur web**) qui envoie une **requête HTTP** à un serveur web pour demander une **ressource** (page HTML, image, fichier, script…).

Dans le cadre de Webserv :

- le **client** se connecte au serveur Webserv via le réseau
- il envoie une requête
- il attend une réponse HTTP

## Requête HTTP

> Une **requête HTTP** est un **message textuel** envoyé par un client à un serveur pour demander ou envoyer des données, structuré d'un manière précise.

Elle commence par une **ligne de requête** :

```
GET /index.html HTTP/1.1
```

Puis vient une série d’**en-têtes** contenant diverses informations comme le nom de l'hôte ou les préférences de langue par exemple (`Host`, `User-Agent`, etc.), une ligne vide, et parfois un **corps**, c'est à dire du contenu (ex : dans une requête `POST`).

Le serveur lit la requête, la comprend, et envoie une **réponse HTTP** structurée.

***-> Les requêtes HTTP sont abordées en détail dans le fichier `HTTP1.1.fr.md`***

## HTML (HyperText Markup Language)

> Le **HTML** est un **langage de balisage** utilisé pour structurer et afficher les pages web.

Un fichier HTML est une **ressource** que le serveur peut envoyer au client.

Exemple simple :

```html
<html>
  <head><title>Ma page</title></head>
  <body><h1>Bonjour !</h1></body>
</html>
```

Le serveur Webserv n’analyse pas le HTML : **il l’envoie tel quel**. C’est le navigateur qui l’interprète.

## Socket

> Une **socket** est une **interface logicielle** (une *portion de code* à l'intérieur d'un programme) qui permet à un programme de **communiquer via le réseau**, en s'appuyant sur des protocoles comme UDP ou TCP.

Dans Webserv :

- le serveur crée une socket avec `socket()`
- il l’associe à une adresse/port avec `bind()`
- il la met en écoute avec `listen()`
- puis il accepte les connexions entrantes avec `accept()`

En C/C++, une socket est représentée par un **descripteur de fichier** (un int), ce qui permet d’utiliser les fonctions classiques comme `read()` et `write()` pour communiquer.

## Port

> Un **port** est un **numéro logique** utilisé pour identifier **une application** sur une machine.

- Le **port 80** est le **port standard du protocole HTTP**
- Le **port 443** est celui de **HTTPS**
- Le **port 8080** est souvent utilisé comme **alternative au port 80**, surtout :
    - quand on n'a pas les droits root
    - pour lancer un serveur de test local

>  Le serveur Webserv écoute sur un port (ex : `8080`) pour recevoir les connexions HTTP.

## TCP (Transmission Control Protocol)

> **TCP** est un **protocole de transport fiable**, utilisé pour transmettre des données entre deux machines sur un réseau, en garantissant que les données :

- arrivent **dans le bon ordre**
- ne sont **pas perdues**
- ne sont **pas dupliquées**

Contrairement à d'autres protocoles comme UDP (qui est rapide mais non fiable), TCP établit une **connexion stable** (appelée session) entre deux points avant de transmettre les données.

HTTP repose sur TCP, ce qui signifie que :

- une requête HTTP circule **dans un flux TCP**
- le serveur Webserv utilise une **socket TCP** pour recevoir les requêtes
- les clients peuvent compter sur une transmission **complète et ordonnée** des messages

> Grâce à TCP, le serveur peut lire une requête HTTP **de manière sûre**, sans devoir vérifier manuellement la cohérence ou l'ordre des paquets.

## UDP (User Datagram Protocol)

> **UDP** est un **protocole de transport rapide mais non fiable**, utilisé pour envoyer des **petits paquets de données** sans établir de connexion préalable.

Contrairement à TCP, UDP ne garantit pas :

- que les paquets arrivent dans le bon ordre
- qu’ils arrivent tout court (pertes possibles)
- qu’ils ne soient pas dupliqués

UDP est donc :

- **plus léger**
- **plus rapide**
- mais aussi **moins fiable**

Chaque message envoyé est appelé un **datagramme**, indépendant des autres. UDP ne propose **aucun contrôle d’erreur ni de retransmission**.

Ce protocole est souvent utilisé pour :

- les jeux en ligne
- les vidéos en streaming en temps réel
- les DNS
- les applications qui tolèrent la perte d’un peu de données

> En résumé, **UDP sacrifie la fiabilité pour la vitesse**, ce qui le rend adapté aux communications **où la réactivité est plus importante que la perfection**.

## CGI (Common Gateway Interface)

> Le **CGI** est une **interface standardisée** qui permet à un serveur web d’**exécuter un programme externe** (comme un script PHP ou Python) et de **renvoyer sa sortie comme réponse HTTP**.

Webserv doit :

- détecter qu’un fichier est un script CGI
- faire un `fork()`
- exécuter le script avec `execve()`
- lire sa sortie (via un pipe)
- envoyer cette sortie comme réponse HTTP

***-> Les CGI sont abordées en détail dans le fichier `CGI.fr.md`***


---

# Fonctions autorisées

Le sujet de Webserv autorise d'utiliser tout ce qui est compatible C++98, sans Boost ou autre bibliothèque externe, et il cite de nombreuses fonctions utilise, qu'on peut regrouper par catégories :

## Gestion de processus et exécution

> Ces fonctions permettent de **lancer des programmes externes (comme les CGI)**, de **gérer les processus enfants**, ou de **rediriger leurs entrées/sorties**.

| Fonction   | Rôle principal                                       |     |
| ---------- | ---------------------------------------------------- | --- |
| `execve`   | Exécute un programme externe (CGI)                   |     |
| `fork`     | Crée un processus fils (utilisé pour CGI uniquement) |     |
| `waitpid`  | Attend la fin d’un processus fils                    |     |
| `kill`     | Envoie un signal à un processus                      |     |
| `signal`   | Définit le comportement pour un signal (ex : SIGINT) |     |
| `errno`    | Variable globale contenant le code d'erreur système  |     |
| `strerror` | Donne le message lisible associé à `errno`           |     |

## Redirections et duplication de descripteurs

> Utilisées pour **rediriger l’entrée/sortie** d’un processus (exécution CGI, gestion des pipes).

|Fonction|Rôle principal|
|---|---|
|`dup`|Duplique un descripteur|
|`dup2`|Duplique vers un descripteur précis|
|`pipe`|Crée une paire de descripteurs pour la communication|

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

## Multiplexage I/O — **Gérer plusieurs connexions à la fois**

> Ces fonctions permettent de **surveiller plusieurs sockets en même temps** pour savoir laquelle est prête à lire ou écrire, sans thread.

| Interface | Fonctions associées                                    |
| --------- | ------------------------------------------------------ |
| `select`  | Méthode portable mais limitée (1024 fd max)            |
| `poll`    | Plus souple que `select`, supporte plus de fd          |
| `epoll`   | Pour Linux : `epoll_create`, `epoll_ctl`, `epoll_wait` |
| `kqueue`  | Pour macOS : `kqueue`, `kevent`                        |

## Résolution de noms et gestion réseau

> Ces fonctions servent à **traduire des noms en adresses IP**, configurer les ports, etc.

|Fonction|Rôle principal|
|---|---|
|`getaddrinfo`|Résout un nom d’hôte (ex : `localhost` → IP)|
|`freeaddrinfo`|Libère la mémoire allouée par `getaddrinfo`|
|`getprotobyname`|Récupère le numéro d’un protocole (ex : "tcp")|
|`htons`, `htonl`|Convertit des entiers en format réseau|
|`ntohs`, `ntohl`|Convertit des entiers **depuis** le format réseau|

## Accès fichiers et systèmes de fichiers

> Pour **servir des fichiers HTML, images, etc.**, gérer les `stat`, ouvrir des fichiers ou des répertoires.

| Fonction                         | Rôle principal                                 |
| -------------------------------- | ---------------------------------------------- |
| `access`                         | Vérifie l’existence ou les droits d’un fichier |
| `stat`                           | Récupère les métadonnées d’un fichier          |
| `open`, `read`, `write`, `close` | Ouvrir, lire, écrire, fermer un fichier        |
| `chdir`                          | Change le répertoire courant (ex : CGI)        |
| `opendir`, `readdir`, `closedir` | Gérer les répertoires                          |


---


# Cycle de vie d’une connexion HTTP dans Webserv

## 1. **Initialisation du serveur**

- `socket()` → crée une socket d’écoute (`SOCK_STREAM`)
- `setsockopt()` → active `SO_REUSEADDR`
- `bind()` → lie la socket à un `host:port`
- `listen()` → met la socket en écoute
- `epoll_create()` / `poll()` → prépare la boucle événementielle

## 2. **Attente de connexions (événement `EPOLLIN` ou `POLLIN`)**

- `accept()` → accepte une connexion entrante
- `epoll_ctl(ADD)` → ajoute la nouvelle socket client à la surveillance
- socket client configurée **non bloquante**

## 3. **Réception de la requête HTTP**

- `recv()` → lit la requête depuis la socket client
- `parse()` → analyse :
    - méthode (GET, POST…)
    - URI / chemin
    - headers (Host, Content-Length…)
    - corps éventuel (POST)        

## 4. **Traitement de la requête**

- **Fichier statique :**
    - `access()` → vérifie existence et droits
    - `stat()` → lit taille/type
    - `open()` + `read()` → lit le fichier demandé
    - construit une **réponse HTTP complète**
        
- **Script CGI :**
    - `fork()` → nouveau processus
    - `pipe()` / `socketpair()` → communication avec le CGI
    - `dup2()` → redirige `stdin` / `stdout`
    - `execve()` → exécute le script
    - `waitpid()` → attend la fin
    - `read()` la sortie CGI
    - parse et **injecte dans la réponse HTTP**

## 5. **Envoi de la réponse**

- `send()` → envoie l’en-tête + corps de la réponse HTTP
- (éventuellement, bascule du `EPOLLIN` au `EPOLLOUT` si non bloquant)

## 6. **Fermeture de la connexion**

- `close()` → ferme la socket client (ou `epoll_ctl(DEL)`)
- (ou **garde-alive** selon `Connection: keep-alive` et timeout)

## Résumé du chemin de traitement

```text
client connect → accept() → recv() → parse()
          ↓
  [fichier statique]         [script CGI]
        ↓                          ↓
   access/stat/open         fork → execve
        ↓                          ↓
      read()                      pipe/read
        ↓                          ↓
   HTTP response           HTTP response
          ↓
        send()
          ↓
        close()
```


---

# Multiplexage

## Définition du Multiplexage (ou **I/O Multiplexing**)

> Le **multiplexage** désigne la capacité d’un programme à **surveiller plusieurs entrées/sorties à la fois**, sans bloquer, et à réagir **dès qu’au moins l’une d’elles devient active** (par exemple : une socket prête à lire ou écrire).

Plutôt que de faire :

```cpp
read(socket1);
read(socket2);
read(socket3);
```

et **attendre bloqué sur chaque socket**, on demande au système :

> « Préviens-moi dès que **l’une d’elles** a quelque chose à dire. »

## Muliplexage dans Webserv

Le serveur doit :

- écouter plusieurs clients en même temps
- ne pas bloquer sur une socket inactive
- répondre à chaque client dès qu’il y a des données

Le **multiplexage** permet de faire **tout cela avec un seul thread/processus**, de manière **fluide et réactive**.

>Il est en effet explicitement interdit dans le sujet d'employer du multithreading avec `pthread()` ou du multiprocessus avec `fork()` à cet effet ) : 
>*"Vous ne pouvez pas utiliser fork pour autre chose que CGI (comme PHP ou Python, etc)."*

## Outils typiques de multiplexage

>Le sujet laisse le choix parmi plusieurs mécanismes pour faire du multiplexage. Il faut en choisir **un seul** et t’y tenir. Ne **jamais** appeler `read()` ou `write()` sans qu’un de ces mécanismes ait validé que le FD est prêt.

|Mécanisme|Systèmes compatibles|Niveau|
|---|---|---|
|`select()`|POSIX (Linux, macOS…)|Basique|
|`poll()`|POSIX (Linux, macOS…)|Moyen|
|`epoll()`|Linux uniquement|Avancé|
|`kqueue()`|BSD/macOS uniquement|Avancé|

### 1. `select()`

> Cette fonction se base sur des **ensembles statiques (`fd_set`)** et des **macros (`FD_SET`, etc.)**

**Avantages :**
- Très simple à comprendre
- Universel (présent partout)
**Inconvénients :**
- Limité à **1024 FDs** (`FD_SETSIZE`)
- Peu performant quand beaucoup de sockets (scan linéaire)
- Syntaxe un peu rigide

### 2. `poll()`

> Cette fonction est basée sur un **tableau dynamique de `pollfd`**. C'est le meilleur compromis pour avoir un code 100% portable.

**Avantages :**
- Moins limité que `select()` (plus de 1024 FDs)
- Interface plus flexible
- Disponible sur tous les systèmes POSIX

**Inconvénients :**
- Toujours un **scan linéaire** (pas optimal en très grand nombre de connexions)
- Requiert de **reconstruire le tableau à chaque tour**

### 3. `epoll()` (Linux uniquement)

> Interface **événementielle** et très performante (non linéaire)

**Avantages :**
- Très performant : O(1) pour ajout/retrait/attente
- Idéal pour des **milliers de connexions simultanées**
- Ne scanne que les FDs prêts

**Inconvénients :**
- Spécifique à **Linux**
- API un peu plus complexe (`epoll_create`, `epoll_ctl`, `epoll_wait`)
- Non portable

### 4. `kqueue()` (macOS / BSD)

> Équivalent d’`epoll` pour les systèmes BSD/macOS

**Avantages :**
- Très performant (comme `epoll`)
- Peut gérer d'autres événements (fichiers, timers, signaux…)

**Inconvénients :**
- Spécifique à macOS et BSD
- Syntaxe un peu plus verbeuse (`kqueue`, `kevent`, etc.)

### Contraintes imposées pour le multiplexage

Le serveur **Webserv** doit fonctionner de manière **non bloquante**, en utilisant **un seul mécanisme de multiplexage** (`poll()`, `select()`, `epoll()`, ou `kqueue`) pour **gérer toutes les opérations I/O**, y compris :

- la **socket d’écoute**
- les **sockets clients**
- les **pipes CGI** éventuels

#### **Règles obligatoires :**

- Utiliser **une seule boucle principale**, avec **un seul appel à `poll()` (ou équivalent choisi)** par itération.
- Ce `poll()` doit surveiller **en même temps** :
    - les sockets **prêtes à lire** (`POLLIN`)
    - les sockets **prêtes à écrire** (`POLLOUT`)
- Ne **jamais faire de `read()` / `recv()` ou `write()` / `send()`** sans que `poll()` (ou équivalent) ait signalé que le descripteur est prêt.
- Il est **strictement interdit** de vérifier la valeur de `errno` après un appel à `read()` / `write()` pour gérer des cas comme `EAGAIN` → cela doit être évité grâce à `poll()`.

>**Exception : lecture du fichier de configuration** :
>On peut lire le fichier `.conf` **sans `poll()`**, de manière bloquante : cette contrainte ne s’applique **que** aux opérations réseau et sockets.

#### Précisions pour `select()`

Si on choisit d’utiliser **`select()`** comme mécanisme de multiplexage dans Webserv, on peut (et on doit) utiliser les macros suivantes pour **gérer les sockets surveillées** :

- `FD_ZERO` : vide un ensemble de descripteurs
- `FD_SET` : ajoute un descripteur à surveiller
- `FD_CLR` : retire un descripteur
- `FD_ISSET` : vérifie si un descripteur est prêt (lecture/écriture)

>Ces macros sont **essentielles avec `select()`**.
>Elles ne s’utilisent **pas avec `poll()` ou `epoll()`**.


---

# Telnet et NGINX

`telnet` et `nginx` sont des outils en ligne de commande (_CLI — Command Line Interface_) utiles pour comprendre et tester Webserv. Ils peuvent servir à :

- tester les requêtes HTTP **"à la main"**, en combinant `telnet` et `nginx`, pour comprendre ce que contient une requête ou une réponse HTTP, comment le serveur réagit à une mauvaise requête, etc.
- tester son propre serveur Webserv en lui envoyant des requêtes avec `telnet`

## Telnet

> `telnet` est un **client réseau en ligne de commande** qui permet d’**ouvrir une connexion TCP** vers un hôte et un port.  
> Il est très utile pour **envoyer manuellement des requêtes HTTP** et observer **la réponse brute** du serveur.

Lancer `telnet` :

```bash
telnet <host> <port>

# par exemple :
telnet localhost 8080    # port par défaut sur macOS
telnet localhost 80      # port par défaut sur Linux
```

Une fois `telnet` lancé, on dispose d’une invite dans laquelle on peut taper une **requête HTTP ligne par ligne**, suivie d’une **ligne vide** pour signaler la fin de la requête (taper deux fois Entrée).

Exemple de requête HTTP dans `telnet` :

```
GET / HTTP/1.1
Host: localhost
```

Si `nginx` (ou un autre serveur, comme Webserv) écoute sur le port visé, il recevra la requête et **renverra une réponse HTTP** que tu verras s’afficher directement dans le terminal.

## NGINX

> `nginx` est un **serveur HTTP performant et populaire**, souvent utilisé en production.  
> Il constitue une référence utile pour comprendre **comment un vrai serveur web réagit** aux requêtes HTTP.

Dans le contexte de Webserv, `nginx` sert à :

- Comparer le **comportement de son propre serveur** à celui de NGINX (y compris lors de la correction : NGINX sert de référence)
- Observer les **réponses à des cas d’erreur** (ex : fichier introuvable, méthode non autorisée)
- Étudier la **structure d’un fichier de configuration** (`nginx.conf`)
- Servir de **modèle** pour écrire son propre fichier `.conf`

Lancer `nginx` :

```bash
brew services start nginx    # sur macOS (écoute par défaut sur le port 8080)
sudo systemctl start nginx   # sur Linux (écoute par défaut sur le port 80)
```

Vérifier qu’il est actif :

```bash
lsof -i :8080    # macOS
lsof -i :80      # Linux
```


---

# Précisions diverses

Le sujet de Webserv impose plusieurs contraintes et pose plusieurs attentes :

## Vous ne pouvez pas exécuter un autre serveur web

Signifie clairement que le programme `webserv` doit être le **seul serveur HTTP actif** pendant l’évaluation : pas le droit de s’appuyer sur un autre serveur existant (comme **nginx**, **Apache**, ou **python -m http.server**).

> On doit  **coder soi-même** tout le fonctionnement d’un serveur HTTP :  
> ouvrir un port, écouter, accepter des connexions, lire la requête, la parser, et générer une réponse.  
> **Pas question de sous-traiter ça** à un autre serveur ou d’en faire un simple proxy.

**Pas autorisé :**

- Lancer **nginx** ou **Apache** pour répondre à la place de Webserv
- Utiliser un programme ou une librairie externe qui **fait le travail de serveur à sa place**
- Déléguer la gestion des connexions, requêtes ou réponses à un **autre binaire ou processus serveur**
- Faire des appels à des outils comme `curl localhost:8080 | python -m http.server` dans le code

**Autorisé et recommandé :**

- Exécuter des **scripts CGI** (comme `./my_script.py`) depuis Webserv
- Comparer le comportement de Webserv avec **nginx** pour tester
- Regarder le comportement d’un autre serveur pour **debugger ou t’inspirer**
- Utiliser des outils clients (`telnet`, `curl`, navigateur, etc.) pour tester le serveur Webserv

## Votre serveur ne doit jamais bloquer

 Le serveur doit pouvoir **gérer plusieurs clients simultanément**, **sans jamais être bloqué** par l’un d’eux, et **répondre de manière correcte et propre** dans tous les cas. Le serveur **doit rester réactif en toutes circonstances**, même si un client :
 
- est lent
- n’envoie pas toute sa requête
- garde la connexion ouverte sans rien faire

>Ne **jamais utiliser `read()` ou `accept()` de manière bloquante**.
>Gérer les connexions via **`select`, `poll`, `epoll`, ou `kqueue`**, pour ne traiter que les sockets **prêtes**.

**Si un client pose problème :**

- Le renvoyer proprement avec une réponse HTTP adaptée :
    - `408 Request Timeout` → s’il ne fait rien
    - `400 Bad Request` → si la requête est malformée
    - `413 Payload Too Large` → si le corps dépasse la limite
    - `405 Method Not Allowed` → méthode non prise en charge
- Et ensuite **fermer la connexion** sans bloquer le reste du serveur.

## Votre serveur doit être compatible avec le navigateur web de votre choix

Le serveur **Webserv** doit pouvoir être testé depuis **un vrai navigateur web** (comme Chrome, Firefox, Safari…), et celui-ci doit être capable de :

- **envoyer une requête HTTP valide** (ex : en tapant `http://localhost:8080`)
- **recevoir et afficher correctement** la réponse du serveur

> Le serveur doit être **assez conforme au protocole HTTP/1.1** pour être **compris et utilisé directement par un navigateur moderne**.  
> Il doit **répondre proprement** à une requête standard du navigateur, sans bug ni erreur visible.

**Cela implique :**

- Le serveur doit **respecter la norme HTTP/1.1** (RFC 2616)
- Il doit **répondre aux en-têtes obligatoires** (`Content-Length`, `Content-Type`, etc.)
- Il doit **gérer les requêtes GET (au minimum)** correctement
- Il doit **retourner une réponse bien formée** (ligne de statut + en-têtes + ligne vide + corps)
- Pas de comportement bloquant ni de réponse incomplète

Mais dans tous les cas, c'est le comportement de NGINX qui sert de référence pour savoir si le serveur Webserv se comporte comme attendu.

## Votre serveur doit avoir des pages d’erreur par défaut si aucune n’est fournie

Si une requête produit une erreur (comme `404 Not Found`, `403 Forbidden`, `500 Internal Server Error`, etc.), et que le fichier de configuration ne spécifie pas de page personnalisée, alors le serveur doit quand même renvoyer une page HTML lisible par le client.

> Si aucune page d’erreur personnalisée n’est fournie dans la configuration, **Webserv doit générer une page HTML d’erreur générique** pour informer correctement le client du problème. Cela s’applique à tous les **codes d’erreur HTTP** (au moins les plus courants : `400`, `403`, `404`, `500`, etc.)

**Exemple :**

Si le fichier `.conf` **ne contient pas** :

```nginx
error_page 404 /404.html;
```

Alors Webserv doit renvoyer **une page HTML générique par défaut**, comme :

```html
<!DOCTYPE html>
<html>
<head><title>404 Not Found</title></head>
<body><h1>404 Not Found</h1></body>
</html>
```

## Vous ne pouvez pas utiliser fork pour autre chose que CGI (comme PHP ou Python, etc)

Il est interdit d'utiliser `fork()`, à part pour exécuter un script CGI (ex : Python, PHP) dans la partie bonus de Webserv.

>Il ne faut **jamais utiliser `fork()` pour gérer un client, créer un thread, ou traiter une requête classique**.

## Vous devriez pouvoir servir un site web entièrement statique

Servir un site **statique** signifie que **Webserv n’a pas besoin d’exécuter du code serveur ni de générer du contenu dynamique**.

>**Pour la partie obligatoire** de Webserv, on se concentre principalement sur : **lire, interpréter, et envoyer des fichiers existants**, comme le ferait un serveur `nginx` minimal. Webserv doit toutefois être capable d’exécuter un script (par exemple Python) via CGI (dans la partie obligatoire), mais pas besoin de parser ou interpréter soi-même le Python ou PHP. C’est l’interpréteur (python, php, etc.) qui s’en charge.

Le serveur doit savoir :

- Servir n’importe quel fichier accessible dans un `root` :
    - `.html`, `.css`, `.js`, `.png`, `.jpeg`, `.pdf`, etc.
- Envoyer les **bons en-têtes** (`Content-Type`, `Content-Length`, `Connection`, etc.)
- Gérer correctement les erreurs (`404`, `403`, `500`, etc.)
- Rediriger ou compléter l’URL si besoin (ex : `/dossier` → `/dossier/` + `index.html`)

Mais pas de :

- Base de données, back-end, SQL…
- Sessions, cookies, formulaires avec traitement
- Authentification, login, registre…
- Frameworks comme Laravel, Django, Express, etc

## Le client devrait pouvoir téléverser des fichiers

Le Webserv doit permettre à un client (navigateur, curl...) d’**envoyer un fichier**,  
qui sera sauvegardécôté serveur. C’est une fonctionnalité essentielle de la méthode **`POST`**, exigée dans la partie obligatoire.

En pratique :

- Le client envoie une requête `POST` contenant :
    - un en-tête `Content-Type: multipart/form-data` ou `application/octet-stream` par exemple
    - un corps contenant le **fichier à téléverser**
- Le serveur doit :
    - **lire le corps** de la requête
    - **enregistrer le fichier** dans un dossier configuré (ex: `/uploads`)
    - renvoyer une **réponse HTTP 201 Created** ou `200 OK`

## Votre serveur doit pouvoir écouter sur plusieurs ports (cf. Fichier de configuration)

Le serveur Webserv doit être capable de :

- **écouter sur plusieurs ports différents en même temps**
- selon ce qui est spécifié dans le **fichier de configuration**

>Cela implique de :
> - Gérer **plusieurs sockets de type "listen"**
> - Les différencier selon le port (et potentiellement l'adresse IP)
> - Faire le lien entre **socket → configuration serveur**

**Exemple dans un fichier `.conf` :**

```nginx
server {
    listen 8080;
    root /var/www/site1;
}

server {
    listen 9090;
    root /var/www/site2;
}
```

**Le serveur Webserv doit alors :**

- créer **une socket d’écoute pour chaque port** (`socket()`, `bind()`, `listen()`)
- les **ajouter toutes à `poll()` (ou équivalent)** dans la boucle principale
- détecter sur quel port arrive chaque connexion    
- et diriger la requête vers la bonne configuration `server {}`

## Pour macOS seulement

Sur **macOS**, le comportement de `write()` en mode non bloquant **n’est pas strictement identique** à celui d’autres systèmes Unix/Linux.
Cela peut provoquer des **blocages involontaires**, même si tu utilises `poll()` correctement.

>**La solution : `fcntl()`**
>`fcntl()` est une fonction système POSIX qui permet de **contrôler ou modifier le comportement d’un descripteur de fichier** (file descriptor — `fd`). Elle est très polyvalente : elle permet de **lire, écrire ou modifier des options** sur un fichier, une socket, un pipe, etc.

```
#include <fcntl.h>

int fcntl(int fd, int cmd, ... /* arg */ );
```

- `fd` → le descripteur concerné (socket, fichier, pipe, etc.)
- `cmd` → l'action à effectuer (ex : F_GETFL, F_SETFL, etc.)
- `arg` → paramètre supplémentaire selon la commande

Le sujet autorise donc à utiliser la fonction système `fcntl()` pour **modifier le comportement des descripteurs**. Mais limité à  :

- `F_SETFL` + `O_NONBLOCK` 

	- Permet de dire à un descripteur : **« ne jamais bloquer en lecture/écriture »**
	- Indispensable pour le multiplexage (`poll`, `select`, etc.)

```cpp
int flags = fcntl(fd, F_GETFL, 0);
fcntl(fd, F_SETFL, flags | O_NONBLOCK);
```

- `F_SETFD` + `FD_CLOEXEC`

	- Marque le descripteur pour qu’il soit **automatiquement fermé dans un processus `execve()`**
	- Évite que les FDs **soient hérités par les scripts CGI**, ce qui causerait des bugs

```cpp
fcntl(fd, F_SETFD, FD_CLOEXEC);
```


---
