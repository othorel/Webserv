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

---
### execve()

```cpp
#include <unistd.h>

int execve(const char *pathname, char *const argv[], char *const envp[]);
```

- `pathname` : chemin vers le fichier exécutable
- `argv` : tableau de chaînes d’arguments terminé par `NULL`
- `envp` : tableau de chaînes d’environnement terminé par `NULL`

- **Retour :**
    - Ne retourne jamais en cas de succès (le processus est remplacé)
    - `-1` en cas d’échec, avec `errno` défini

La fonction `execve()` remplace le **processus courant** par un **nouveau programme** spécifié par `pathname`. C’est l’appel système de bas niveau derrière toute la famille `exec()` (comme `execl`, `execvp`, etc.).

#### Cas d’usage typiques

- Lancer un **script CGI** depuis un processus enfant
- Exécuter un **programme externe** (comme `/usr/bin/php`)
- Remplacer un processus par un autre après un `fork()`

#### Fonctionnement

`execve()` ne crée pas un nouveau processus : il **remplace** l’image du processus courant par celle du programme cible.  
Tous les segments mémoire, pile et code sont écrasés. Les **descripteurs de fichiers** ouverts restent ouverts (sauf s’ils ont le flag `FD_CLOEXEC`).

En cas d’échec, le processus courant continue l’exécution après l’appel avec un retour `-1`.

#### Exemple d'utilisation

```cpp
#include <unistd.h>
#include <stdio.h>

int main()
{
    char *args[] = {"/bin/ls", "-l", NULL};
    char *env[] = {NULL};

    execve("/bin/ls", args, env);
    // Si execve échoue
    perror("execve a échoué");
    return 1;
}
```

#### Gestion des erreurs

Retourne `-1` et définit `errno` :
- `ENOENT` : fichier inexistant
- `EACCES` : permissions insuffisantes
- `ENOMEM` : mémoire insuffisante
- `EFAULT` : adresse invalide

#### Dans Webserv

Dans Webserv, vous utiliserez `execve()` pour :  
✅ **Lancer les scripts CGI** (ex. Python, PHP) dans les processus enfants  
✅ **Remplacer le processus enfant par l’exécutable cible**  
✅ **Transmettre les variables d’environnement HTTP** via le tableau `envp`


---
### fork()

```cpp
#include <unistd.h>

pid_t fork(void);
```

- **Retour :**
    - `> 0` dans le processus **parent**, avec le PID du processus enfant.
    - `0` dans le processus **enfant**.
    - `-1` en cas d’erreur, et `errno` est défini.

La fonction `fork()` crée un **nouveau processus** en dupliquant le processus appelant. Le processus enfant reçoit une copie quasi-identique du processus parent, y compris des descripteurs de fichiers ouverts, mais avec un espace mémoire distinct.

#### Cas d’usage typiques

- Lancer un script CGI dans un **processus enfant**.
- Préparer une exécution avec `execve()` dans le processus enfant.
- Implémenter une gestion **manuelle des processus** dans un serveur Web.
#### Fonctionnement

Le processus enfant hérite :

- de l’environnement et de la mémoire du parent
- des descripteurs de fichiers
- mais a un **PID unique** et son propre espace d’adressage.

Après le `fork()`, les deux processus continuent à s’exécuter à partir de l’instruction suivante. Il est donc nécessaire de différencier les rôles via la valeur de retour.

#### Exemple d'utilisation

```cpp
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>

int main()
{
    pid_t pid = fork();

    if (pid < 0)
        perror("Échec du fork");
    else if (pid == 0)
        printf("Je suis le processus enfant\n");
    else
        printf("Je suis le processus parent, PID de l’enfant : %d\n", pid);
    return 0;
}
```

#### Gestion des erreurs

Retourne `-1` en cas d’échec et définit `errno` :
- `EAGAIN` : Trop de processus créés ou limite de ressources atteinte
- `ENOMEM` : Mémoire insuffisante pour dupliquer le processus

#### Dans Webserv

Dans Webserv, vous utiliserez `fork()` pour :  
✅ **Créer un processus enfant** dédié à l’exécution d’un script CGI  
✅ **Isoler le traitement** de la requête dans un processus séparé  
✅ **Préparer un appel à `execve()`**, sans affecter le processus principal

---
### waitpid()

```cpp
#include <sys/types.h>
#include <sys/wait.h>

pid_t waitpid(pid_t pid, int *status, int options);
```

- `pid` :
    - `> 0` : attend ce processus précis
    - `-1` : attend n’importe quel processus enfant
- `status` : pointeur vers un entier pour stocker le **code de retour**
- `options` : options comme `WNOHANG` (ne pas bloquer) ou `WUNTRACED`
- 
- **Retour :**
    - PID du processus terminé
    - `0` si `WNOHANG` est spécifié et aucun enfant n’est encore terminé
    - `-1` en cas d’erreur, avec `errno` défini

La fonction `waitpid()` permet au **processus parent d’attendre la fin d’un processus enfant**, ou de vérifier son état sans se bloquer. Elle est indispensable pour **éviter les processus zombies** (zombie processes).

#### Cas d’usage typiques

- Récupérer la **fin d’un script CGI** lancé avec `fork()`
- Nettoyer les processus terminés pour éviter les zombies
- Gérer plusieurs enfants de façon sélective ou non-bloquante

#### Fonctionnement

`waitpid()` suspend le processus appelant jusqu’à ce qu’un **processus enfant spécifié se termine** ou change d’état (selon les options).  
Le paramètre `status` peut être analysé avec des macros :

- `WIFEXITED(status)` : le processus s’est terminé normalement
- `WEXITSTATUS(status)` : récupère son **code retour**
- `WIFSIGNALED(status)` : terminé par un signal
- `WTERMSIG(status)` : quel signal l’a arrêté

#### Exemple d'utilisation

```cpp
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>

int main()
{
    pid_t pid = fork();

    if (pid == 0)
        _exit(42); // Enfant quitte avec code 42
    else {
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status))
            printf("L’enfant s’est terminé avec le code %d\n", WEXITSTATUS(status));
    }
    return 0;
}
```

#### Gestion des erreurs

Retourne `-1` et définit `errno` :

- `ECHILD` : aucun processus enfant à attendre
- `EINVAL` : options invalides
- `EINTR` : interrompu par un signal

#### Dans Webserv

Dans Webserv, vous utiliserez `waitpid()` pour :  
✅ **Attendre la fin du script CGI** exécuté via `fork()`  
✅ **Récupérer son code de sortie** (utile pour détecter une erreur du script)  
✅ **Éviter les zombies** en collectant proprement les processus enfants terminés

---
### kill()

```cpp
#include <signal.h>

int kill(pid_t pid, int sig);
```

- `pid` :
    - `> 0` : envoie le signal au processus identifié par ce PID
    - `0` : envoie le signal à tous les processus du même groupe que l’appelant
    - `-1` : envoie le signal à **tous les processus** que l’utilisateur a le droit de signaler
- `sig` : signal à envoyer (ex. `SIGTERM`, `SIGKILL`, `SIGINT`)

- **Retour :**
    - `0` en cas de succès
    - `-1` en cas d’échec, avec `errno` défini

La fonction `kill()` permet d’**envoyer un signal à un ou plusieurs processus**. Le signal peut être utilisé pour **terminer**, **interrompre**, ou **notifier** un processus. Contrairement à son nom, `kill()` ne tue pas forcément le processus — cela dépend du signal.

#### Cas d’usage typiques

- Arrêter un **processus CGI bloqué** depuis un processus parent
- Envoyer `SIGTERM` pour une **terminaison propre**
- Vérifier si un processus existe (`kill(pid, 0)`)

#### Fonctionnement

Les signaux sont une méthode de **communication inter-processus**. Le signal est envoyé au processus désigné. Celui-ci peut :

- réagir via un gestionnaire (`signal()` ou `sigaction()`)
- l’ignorer
- ou être tué, selon la nature du signal

Seuls les signaux comme `SIGKILL` et `SIGSTOP` sont **non interceptables**.

#### Exemple d'utilisation

```cpp
#include <signal.h>
#include <unistd.h>
#include <stdio.h>

int main()
{
    pid_t pid = fork();

    if (pid == 0) {
        while (1) {} // boucle infinie
    } else {
        sleep(1);
        kill(pid, SIGTERM); // demande l'arrêt du fils
        printf("SIGTERM envoyé à l’enfant\n");
    }
    return 0;
}
```

#### Gestion des erreurs

Retourne `-1` et définit `errno` :
- `ESRCH` : aucun processus correspondant
- `EPERM` : permission refusée pour envoyer ce signa
- `EINVAL` : signal invalide

#### Dans Webserv

Dans Webserv, vous utiliserez `kill()` pour :  
✅ **Arrêter un processus CGI** qui prend trop de temps  
✅ **Vérifier l’existence d’un processus** avant d’interagir avec lui  
✅ **Envoyer un signal de terminaison** pour libérer les ressources proprement

---
### signal()

```cpp
#include <signal.h>

typedef void (*sighandler_t)(int);
sighandler_t signal(int signum, sighandler_t handler);
```

- `signum` : numéro du **signal** à intercepter (ex. `SIGINT`, `SIGTERM`)
- `handler` : fonction à exécuter lors de la réception du signal, ou bien :
    - `SIG_IGN` : ignorer le signal
    - `SIG_DFL` : comportement par défaut

- **Retour :**
    - Ancien gestionnaire de signal en cas de succès
    - `SIG_ERR` en cas d’erreur, avec `errno` défini

La fonction `signal()` permet de **définir un comportement personnalisé** lorsqu’un signal est reçu par le processus. Elle est utile pour **gérer proprement des interruptions** (par exemple un `Ctrl+C`) ou pour **ignorer certains signaux**.

#### Cas d’usage typiques

- Intercepter `SIGINT` pour fermer proprement le serveur
- Ignorer `SIGPIPE` lors de l’écriture dans un socket fermé
- Exécuter un code spécifique à la réception d’un signal

#### Fonctionnement

Lorsque le processus reçoit un signal correspondant à `signum`, la fonction désignée par `handler` est automatiquement appelée.  
Ce gestionnaire de signal doit être le plus simple possible : les fonctions comme `printf()` ou `malloc()` ne sont **pas sûres** dans ce contexte.

La fonction `signal()` est simple à utiliser mais **moins puissante et plus portable** que `sigaction()`.

#### Exemple d'utilisation

```cpp
#include <signal.h>
#include <unistd.h>
#include <stdio.h>

void handler(int sig)
{
    write(1, "Signal reçu\n", 12);
}

int main()
{
    signal(SIGINT, handler);
    while (1)
        pause(); // attend un signal
    return 0;
}
```

#### Gestion des erreurs

Retourne `SIG_ERR` et définit `errno` :

- `EINVAL` : signal invalide
- `ENOMEM` : ressources insuffisantes pour installer le gestionnaire (rare)

#### Dans Webserv

Dans Webserv, vous utiliserez `signal()` pour :  
✅ **Ignorer certains signaux**, comme `SIGPIPE`, dans les processus enfants  
✅ **Installer un handler temporaire** pour gérer un événement système  
✅ **Contrôler proprement les interruptions** du serveur pendant son exécution

---
### errno

```cpp
#include <cerrno>
```

`errno` est une **variable globale** (de type `int`) définie par la bibliothèque standard. Elle est **modifiée automatiquement** par la majorité des appels système et fonctions de la bibliothèque standard C lorsqu'une erreur se produit.

- Elle **n’est jamais remise à zéro automatiquement**
- Sa valeur **n’est significative que si une fonction a échoué**
- Peut être lue immédiatement après un échec (ex : retour `-1` ou `NULL`)

#### Cas d’usage typiques

- Diagnostiquer la raison d’un **échec d’appel système** (ex. `open()`, `execve()`)
- Afficher un message d’erreur précis avec `perror()` ou `strerror()`
- Traiter différents cas d’erreurs selon la valeur de `errno`

#### Fonctionnement

Après un appel système ou une fonction standard ayant échoué, la variable `errno` contient un **code d’erreur** défini dans `<cerrno>` (ou `<errno.h>` en C). Chaque valeur correspond à une erreur standard (ex : `EACCES`, `ENOENT`, `ENOMEM`...).

Cette variable est **thread-local** sur les systèmes modernes (chaque thread a sa propre copie).

#### Exemple d'utilisation

```cpp
#include <unistd.h>
#include <cerrno>
#include <cstdio>

int main()
{
    if (access("inexistant.txt", F_OK) == -1) {
        printf("Erreur : %d\n", errno); // Affiche le code d’erreur
    }
    return 0;
}
```

#### Gestion des erreurs

Exemples de valeurs de `errno` courantes :

- `EACCES` : permission refusée
- `ENOENT` : fichier ou répertoire inexistant
- `ENOMEM` : mémoire insuffisante
- `EEXIST` : le fichier existe déjà

Utilisez `strerror(errno)` pour obtenir un message d’erreur descriptif, ou `perror()` pour une impression automatique.

#### Dans Webserv

Dans Webserv, vous utiliserez `errno` pour :  
✅ **Identifier la cause exacte** d’une erreur système (exec, socket, etc.)  
✅ **Afficher ou logguer des messages précis** lors de défaillances  
✅ **Adapter le comportement du serveur** en fonction du type d’erreur rencontrée

---
### strerror()

```cpp
#include <cstring>

char *strerror(int errnum);
```

- `errnum` : code d’erreur (souvent la valeur de `errno`)

- **Retour :**
    - Pointeur vers une **chaîne de caractères statique** contenant un message d’erreur lisible
    - `NULL` en cas d’erreur (rare)

La fonction `strerror()` convertit un **code d’erreur** (comme ceux contenus dans `errno`) en une **chaîne explicative** en anglais lisible par un humain. Elle est utile pour **afficher un diagnostic clair** dans les messages d’erreur.

#### Cas d’usage typiques

- Afficher un message d’erreur détaillé après un appel système échoué
- Compléter un message personnalisé avec l’explication du code d’erreur
- Alternative directe à `perror()` pour un **meilleur contrôle de l’affichage**

#### Fonctionnement

`strerror()` prend un entier représentant un code d’erreur et retourne une **chaîne littérale statique** décrivant l’erreur.  
Cette chaîne **ne doit pas être modifiée** ni libérée.  
L’appel peut être non thread-safe car la chaîne retournée est partagée.

#### Exemple d'utilisation

```cpp
#include <unistd.h>
#include <cerrno>
#include <cstring>
#include <cstdio>

int main()
{
    if (access("inexistant.txt", F_OK) == -1)
        printf("Erreur : %s\n", strerror(errno));
    return 0;
}
```

#### Gestion des erreurs

Si le code `errnum` est inconnu, certaines implémentations retournent un message générique (`"Unknown error"`), ou `NULL`.

#### Dans Webserv

Dans Webserv, vous utiliserez `strerror()` pour :  
✅ **Afficher un message compréhensible** à partir d’un code `errno`  
✅ **Logguer les erreurs système** dans les logs du serveur  
✅ **Améliorer le débogage** des appels critiques (open, bind, exec...)

---

