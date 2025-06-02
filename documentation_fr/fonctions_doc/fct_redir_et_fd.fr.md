## Redirections et duplication de descripteurs

> Utilisées pour **rediriger l’entrée/sortie** d’un processus (exécution CGI, gestion des pipes).

|Fonction|Rôle principal|
|---|---|
|`dup`|Duplique un descripteur|
|`dup2`|Duplique vers un descripteur précis|
|`pipe`|Crée une paire de descripteurs pour la communication|

---
### dup()

```cpp
#include <unistd.h>

int dup(int oldfd);
```

- `oldfd` : descripteur de fichier existant

- **Retour :**
    - Nouveau descripteur de fichier (le plus petit disponible)
    - `-1` en cas d’échec, avec `errno` défini

La fonction `dup()` crée une **copie du descripteur de fichier `oldfd`**. Le nouveau descripteur fait référence au **même fichier**, **même position dans le flux**, et **mêmes flags** que l’original (hors `O_CLOEXEC`).

#### Cas d’usage typiques

- Sauvegarder `stdin`, `stdout` ou `stderr` avant redirection
- Restaurer la sortie ou l’entrée d’un processus après exécution
- Cloner un descripteur pour en faire un autre `stdout`, par exemple

#### Fonctionnement

`dup()` retourne un **nouveau descripteur** qui pointe vers **le même fichier ouvert** que `oldfd`. Il commence à chercher à partir de la plus petite valeur disponible.

Les deux descripteurs peuvent être utilisés indépendamment mais partagent le **même offset** (position de lecture/écriture) et l’état du fichier.

#### Exemple d'utilisation

```cpp
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>

int main()
{
    int fd = open("fichier.txt", O_WRONLY | O_CREAT, 0644);
    int sauvegarde_stdout = dup(1); // copie de stdout

    dup2(fd, 1); // redirige stdout vers le fichier

    printf("Ce message va dans le fichier\n");

    dup2(sauvegarde_stdout, 1); // restaure stdout

    printf("Ce message revient dans le terminal\n");
    return 0;
}
```

#### Gestion des erreurs

Retourne `-1` et définit `errno` :
- `EBADF` : `oldfd` n’est pas valide
- `EMFILE` : trop de descripteurs ouverts

#### Dans Webserv

Dans Webserv, vous utiliserez `dup()` pour :  
✅ **Sauvegarder `stdin` et `stdout`** avant de les rediriger vers un pipe ou fichier  
✅ **Rétablir les flux standards** après exécution d’une commande  
✅ **Manipuler des redirections** sans perdre les descripteurs d’origine


---
### dup2()

```cpp
#include <unistd.h>

int dup2(int oldfd, int newfd);
```

- `oldfd` : descripteur de fichier existant
- `newfd` : descripteur cible à utiliser ou remplacer

- **Retour :**
    - `newfd` en cas de succès
    - `-1` en cas d’échec, avec `errno` défini

La fonction `dup2()` crée une **copie du descripteur `oldfd` dans `newfd`**. Si `newfd` est déjà ouvert, il est **automatiquement fermé** avant d’être réutilisé. Cela permet de **rediriger précisément un flux vers un autre**.

#### Cas d’usage typiques

- Rediriger `stdout` vers un fichier (`dup2(fd, 1)`)
- Rediriger `stdin` depuis un pipe (`dup2(pipe[0], 0)`)
- Rebrancher un descripteur spécifique de manière contrôlée

#### Fonctionnement

Si `oldfd` et `newfd` sont égaux, `dup2()` ne fait rien et retourne `newfd`.  
Sinon, `newfd` est fermé s’il est ouvert, puis une copie de `oldfd` est assignée à sa place.  
Le **nouveau descripteur partage le même fichier, offset et flags** que l’original.

Cette fonction est plus **précise** que `dup()` car elle permet d’imposer la valeur du nouveau descripteur.

#### Exemple d'utilisation

```cpp
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>

int main()
{
    int fd = open("sortie.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);

    if (fd == -1)
        return 1;

    dup2(fd, 1); // redirige stdout vers le fichier
    printf("Ce message va dans le fichier sortie.txt\n");

    return 0;
}
```

#### Gestion des erreurs

Retourne `-1` et définit `errno` :
- `EBADF` : `oldfd` invalide ou `newfd` non utilisable
- `EMFILE` : trop de descripteurs ouverts
- `EINTR` : appel interrompu par un signal
    

#### Dans Webserv

Dans Webserv, vous utiliserez `dup2()` pour :  
✅ **Rediriger l’entrée/sortie standard** vers un pipe ou fichier  
✅ **Réaliser des connexions précises** entre descripteurs (ex. entre deux commandes)  
✅ **Remplacer des descripteurs de façon déterministe** lors de l’exécution de CGI ou commandes


---
### pipe()

```cpp
#include <unistd.h>

int pipe(int pipefd[2]);
```

- `pipefd` : tableau de deux entiers
    - `pipefd[0]` : extrémité de **lecture**
    - `pipefd[1]` : extrémité d’**écriture**

- **Retour :**
    - `0` en cas de succès
    - `-1` en cas d’échec, avec `errno` défini

La fonction `pipe()` crée un **canal de communication unidirectionnel** entre deux processus, sous forme de **descripteurs de fichiers**. Les données écrites dans `pipefd[1]` peuvent être lues depuis `pipefd[0]`.

#### Cas d’usage typiques

- Créer une **connexion entre un parent et un enfant** après `fork()`
- Implémenter le **comportement d’un pipe `|`** dans un shell
- Relier la sortie d’une commande à l’entrée d’une autre

#### Fonctionnement

`pipe()` remplit le tableau `pipefd` avec deux descripteurs :
- `pipefd[1]` : on écrit dedans (comme dans `write()`)
- `pipefd[0]` : on lit dedans (comme avec `read()`)

C’est un **mécanisme de communication inter-processus**, typiquement utilisé entre un parent et un enfant après `fork()`.

#### Exemple d'utilisation

```cpp
#include <unistd.h>
#include <stdio.h>

int main()
{
    int pipefd[2];
    pipe(pipefd);

    pid_t pid = fork();

    if (pid == 0) {
        close(pipefd[1]); // enfant lit
        char buffer[100];
        read(pipefd[0], buffer, sizeof(buffer));
        printf("Enfant a reçu : %s\n", buffer);
    } else {
        close(pipefd[0]); // parent écrit
        write(pipefd[1], "Bonjour enfant", 15);
    }
    return 0;
}
```

#### Gestion des erreurs

Retourne `-1` et définit `errno` :
- `EMFILE` : trop de fichiers ouverts
- `EFAULT` : `pipefd` pointe vers un espace mémoire invalide

#### Dans Webserv

Dans Webserv, vous utiliserez `pipe()` pour :  
✅ **Relier deux processus via un tube**, notamment dans l’exécution de commandes enchaînées 
✅ **Capturer la sortie d’un script CGI** pour la rediriger vers le client HTTP  
✅ **Mettre en place une communication sécurisée** et temporaire entre processus

---

