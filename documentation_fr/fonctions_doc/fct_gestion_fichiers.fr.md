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
### access()

```cpp
#include <unistd.h>

int access(const char *pathname, int mode);
```

- `pathname` : chemin du fichier ou répertoire à tester
- `mode` : mode d’accès à tester (`R_OK`, `W_OK`, `X_OK`, `F_OK`)

- **Retour :**
    - `0` si l’accès est autorisé
    - `-1` en cas d’échec, avec `errno` défini

La fonction `access()` permet de **tester les droits d’accès réels** d’un processus à un fichier ou répertoire, selon les droits de l’utilisateur **effectif** (et non les droits effectifs du processus après `seteuid()`).

#### Cas d’usage typiques

- Vérifier l’**existence** d’un fichier (`F_OK`)
- Tester si un fichier est **lisible ou exécutable** avant d’y accéder
- Implémenter des **contrôles de sécurité** pour les scripts CGI ou les fichiers de configuration

#### Fonctionnement

`access()` ne tente pas d’ouvrir le fichier, mais **teste uniquement les permissions**.  
Le paramètre `mode` peut combiner plusieurs flags avec `|` :

- `F_OK` : le fichier existe
- `R_OK` : lecture autorisée
- `W_OK` : écriture autorisée
- `X_OK` : exécution autorisée

#### Exemple d'utilisation

```cpp
#include <unistd.h>
#include <stdio.h>

int main()
{
    if (access("/etc/passwd", R_OK) == 0)
        printf("Fichier lisible\n");
    else
        perror("Impossible d'accéder au fichier");
    return 0;
}
```

#### Gestion des erreurs

Retourne `-1` et définit `errno` :
- `EACCES` : permission refusée
- `ENOENT` : fichier ou chemin inexistant
- `ENOTDIR` : une partie du chemin n’est pas un répertoire
- `ELOOP` / `EFAULT` / `ENOMEM` : autres erreurs système

#### Dans Webserv

Dans Webserv, vous utiliserez `access()` pour :  
✅ **Vérifier si un fichier cible (ex. CGI, page statique) existe et est lisible/exécutable**  
✅ **Implémenter les erreurs 403 / 404 avant d’ouvrir un fichier**  
✅ **Contrôler les permissions de scripts exécutables dans les blocs `location`**


---
### stat()

```cpp
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

int stat(const char *pathname, struct stat *buf);
```

- `pathname` : chemin du fichier à examiner
- `buf` : pointeur vers une structure `stat` à remplir

- **Retour :**
    - `0` en cas de succès
    - `-1` en cas d’échec, avec `errno` défini

La fonction `stat()` permet d’**obtenir les métadonnées d’un fichier**, telles que sa taille, ses permissions, ses dates d’accès/modification, ou encore son type (fichier, répertoire, lien symbolique…).

#### Cas d’usage typiques

- Déterminer si un chemin est un **répertoire ou un fichier ordinaire**
- Lire la **taille d’un fichier** pour préparer une réponse HTTP
- Vérifier les **droits et propriétaires** d’un fichier

#### Fonctionnement

La structure `struct stat` est remplie avec les informations sur le fichier désigné.  
Parmi ses champs les plus courants :

- `st_mode` : type et permissions (à analyser avec `S_ISREG()`, `S_ISDIR()`, etc.)
- `st_size` : taille du fichier en octets
- `st_mtime` : date de dernière modification
- `st_uid`, `st_gid` : UID et GID du propriétaire

#### Exemple d'utilisation

```cpp
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>

int main()
{
    struct stat sb;
    if (stat("fichier.txt", &sb) == 0)
    {
        if (S_ISREG(sb.st_mode))
            printf("C’est un fichier ordinaire de %ld octets\n", sb.st_size);
    }
    return 0;
}
```

#### Gestion des erreurs

Retourne `-1` et définit `errno` :
- `ENOENT` : fichier inexistant
- `ENOTDIR` : une partie du chemin n’est pas un répertoire
- `EACCES` : accès interdit
- `ELOOP`, `EFAULT`, `ENOMEM` : autres erreurs système

#### Dans Webserv

Dans Webserv, vous utiliserez `stat()` pour :  
✅ **Déterminer le type de ressource** (fichier, dossier, lien…) demandée dans l’URL  
✅ **Lire la taille du fichier** pour envoyer le bon `Content-Length`  
✅ **Vérifier si un fichier est exécutable ou non** avant d’en faire un CGI


---
### open(), read(), write(), close()

```cpp
#include <fcntl.h>     // pour open()
#include <unistd.h>    // pour read(), write(), close()

int open(const char *pathname, int flags, ...);
ssize_t read(int fd, void *buf, size_t count);
ssize_t write(int fd, const void *buf, size_t count);
int close(int fd);
```

- `pathname` : chemin du fichier à ouvrir (pour `open()`)
- `flags` : mode d’ouverture (`O_RDONLY`, `O_WRONLY`, `O_RDWR`, `O_CREAT`, `O_TRUNC`, etc.)
- `buf` : tampon mémoire pour lire/écrire
- `count` : nombre d’octets à lire/écrire
- `fd` : descripteur de fichier ouvert

- **Retour :**
    - `open()` : descripteur de fichier ou `-1` en cas d’échec
    - `read()` / `write()` : nombre d’octets lus/écrits ou `-1`
    - `close()` : `0` en cas de succès, `-1` en cas d’erreur

Ces fonctions forment le socle des **opérations de fichiers en bas niveau** sur Unix. Elles permettent d’**ouvrir**, **lire**, **écrire** et **fermer** un fichier, un socket, un pipe ou tout autre descripteur.

#### Cas d’usage typiques

- Lire un **fichier statique** à renvoyer dans une réponse HTTP
- Écrire la **sortie d’un CGI** dans un pipe
- Lire depuis une **requête POST** reçue via un socket

#### Fonctionnement

- `open()` retourne un descripteur de fichier (int ≥ 0) ou `-1`.  
    Il accepte des **options combinées** avec `|` :
    - `O_CREAT` : crée le fichier s’il n’existe pas (nécessite un 3e argument `mode`)
    - `O_TRUNC` : tronque le fichier
    - `O_APPEND` : ajout en fin de fichier
- `read()` lit au plus `count` octets et les place dans `buf`
- `write()` envoie `count` octets de `buf` vers le descripteur
- `close()` libère le descripteur (important pour éviter les fuites)

#### Exemple d'utilisation

```cpp
#include <fcntl.h>
#include <unistd.h>

int main()
{
    int fd = open("fichier.txt", O_RDONLY);
    char buf[1024];
    ssize_t n = read(fd, buf, sizeof(buf));
    write(1, buf, n); // affiche dans stdout
    close(fd);
    return 0;
}
```

#### Gestion des erreurs

- `open()` : `ENOENT`, `EACCES`, `EMFILE`, `ENFILE`
- `read()` / `write()` : `EAGAIN`, `EINTR`, `EBADF`
- `close()` : `EBADF`, `EIO`

#### Dans Webserv

Dans Webserv, vous utiliserez ces fonctions pour :  
✅ **Ouvrir et lire les fichiers demandés par les clients** (HTML, images, etc.)  
✅ **Lire/écrire dans des pipes ou fichiers temporaires** pour les scripts CGI  
✅ **Fermer correctement tous les descripteurs** ouverts pour éviter les fuites mémoire


---
### chdir()

```cpp
#include <unistd.h>

int chdir(const char *path);
```

- `path` : chemin absolu ou relatif du répertoire à définir comme courant

- **Retour :**
    - `0` en cas de succès
    - `-1` en cas d’échec, avec `errno` défini

La fonction `chdir()` modifie le **répertoire de travail courant** du processus appelant. Elle affecte toutes les opérations de fichiers utilisant des **chemins relatifs** après son appel.

#### Cas d’usage typiques

- Changer de répertoire avant l’**exécution d’un script CGI**
- Accéder à un **répertoire racine personnalisé** dans un `location` (type `root` ou `chroot-like`)
- Mettre en œuvre une séparation logique des espaces fichiers

#### Fonctionnement

Une fois `chdir()` appelé avec succès, tout chemin relatif est interprété **par rapport au nouveau répertoire courant**.

Cela ne change rien pour les chemins absolus.  
Le changement de répertoire ne s’applique qu’au **processus courant et ses enfants**.

#### Exemple d'utilisation

```cpp
#include <unistd.h>
#include <stdio.h>

int main()
{
    if (chdir("/tmp") == 0)
        printf("Répertoire changé vers /tmp\n");
    else
        perror("Erreur chdir");
    return 0;
}
```

#### Gestion des erreurs

Retourne `-1` et définit `errno` :
- `ENOENT` : le répertoire n’existe pas
- `EACCES` : permission refusée
- `ENOTDIR` : une composante du chemin n’est pas un répertoire

#### Dans Webserv

Dans Webserv, vous utiliserez `chdir()` pour :  
✅ **Accéder aux fichiers d’un répertoire cible** défini par une directive `root`  
✅ **Exécuter un script CGI depuis son dossier dédié**  
✅ **Isoler les ressources statiques dans un espace de travail sécurisé**


---
### opendir(), readdir(), closedir()

```cpp
#include <dirent.h>

DIR *opendir(const char *name);
struct dirent *readdir(DIR *dirp);
int closedir(DIR *dirp);
```

- `name` : chemin du répertoire à ouvrir
- `dirp` : pointeur vers un objet `DIR` représentant le flux de lecture du répertoire

- **Retour :**
    - `opendir()` : pointeur `DIR *` ou `NULL` en cas d’échec
    - `readdir()` : pointeur `struct dirent *` ou `NULL` (fin ou erreur)
    - `closedir()` : `0` en cas de succès, `-1` en cas d’échec

Ces fonctions permettent de **parcourir le contenu d’un répertoire**, en listant tous les fichiers et sous-répertoires qu’il contient. Elles sont analogues à `fopen()`/`fread()`/`fclose()` mais pour les dossiers.

#### Cas d’usage typiques

- Lister les **fichiers présents dans un dossier** public (autoindex)
- Parcourir les **dossiers CGI ou uploads**
- Vérifier la **présence d’un fichier** spécifique dans un répertoire

#### Fonctionnement

- `opendir()` ouvre un répertoire et retourne un flux `DIR *`
- `readdir()` lit la prochaine **entrée du répertoire**
- `closedir()` libère les ressources du flux

La structure `dirent` contient notamment le champ `d_name` (nom de l’entrée).  
Il faut **boucler avec `readdir()` jusqu’à ce qu’il retourne `NULL`**.

#### Exemple d'utilisation

```cpp
#include <dirent.h>
#include <stdio.h>

int main()
{
    DIR *dir = opendir(".");
    if (!dir)
        return 1;

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
        printf("Nom : %s\n", entry->d_name);

    closedir(dir);
    return 0;
}
```

#### Gestion des erreurs

- `opendir()` : retourne `NULL` ; erreurs possibles : `ENOENT`, `EACCES`, `ENOTDIR`
- `readdir()` : retourne `NULL` à la fin ou en cas d’erreur (vérifiez `errno`)
- `closedir()` : retourne `-1` si échec (ex. pointeur invalide)

#### Dans Webserv

Dans Webserv, vous utiliserez ces fonctions pour :  
✅ **Lister les fichiers disponibles dans un dossier cible** si `autoindex` est activé  
✅ **Générer dynamiquement un listing HTML** des ressources présentes  
✅ **Parcourir un répertoire de scripts CGI, de logs ou de fichiers uploadés**


---

## Structures associées aux fonctions système


### `struct stat` (utilisée avec `stat()`)

```cpp
#include <sys/stat.h>

struct stat {
    dev_t     st_dev;     // ID du périphérique contenant le fichier
    ino_t     st_ino;     // Numéro d'inode
    mode_t    st_mode;    // Type et permissions
    nlink_t   st_nlink;   // Nombre de liens
    uid_t     st_uid;     // UID du propriétaire
    gid_t     st_gid;     // GID du groupe
    off_t     st_size;    // Taille du fichier en octets
    time_t    st_atime;   // Dernier accès
    time_t    st_mtime;   // Dernière modification
    time_t    st_ctime;   // Dernier changement d’état
    ...
};
```

**Utilisation typique dans Webserv :**

- `st_mode` → `S_ISREG()`, `S_ISDIR()` pour tester le type
- `st_size` → taille à inclure dans `Content-Length`
- `st_mtime` → date de dernière modification (header `Last-Modified`)

---
### `struct dirent` (utilisée avec `readdir()`)

```cpp
#include <dirent.h>

struct dirent {
    ino_t          d_ino;       // Numéro d’inode
    off_t          d_off;       // Offset dans le répertoire
    unsigned short d_reclen;    // Longueur de l’enregistrement
    unsigned char  d_type;      // Type de fichier (si supporté)
    char           d_name[256]; // Nom de l’entrée
};
```

**Utilisation typique dans Webserv :**

- `d_name` → nom de chaque fichier ou dossier à afficher dans un `autoindex`
- `d_type` → `DT_REG`, `DT_DIR` (non portable, mais pratique si disponible)

---

## Macros

### **Fichiers et permissions (`stat.h`)**

|Macro|Description|Utilisation dans Webserv|
|---|---|---|
|`S_ISREG(m)`|Fichier ordinaire ?|Servir un fichier statique|
|`S_ISDIR(m)`|Répertoire ?|Gérer l’accès à un répertoire (autoindex)|
|`S_IXUSR`|L’utilisateur peut exécuter ?|Vérifier si un CGI est exécutable|
|`S_IRUSR`|L’utilisateur peut lire ?|Vérifier lecture d’un fichier|
|`S_IWUSR`|L’utilisateur peut écrire ?|Vérifier permission d’écriture|

### **Contrôle d’accès (`unistd.h`)**

|Macro|Description|Utilisation dans Webserv|
|---|---|---|
|`F_OK`|Le fichier existe|`access(path, F_OK)`|
|`R_OK`|Lecture autorisée|Vérifier si un fichier est lisible|
|`W_OK`|Écriture autorisée|Vérifier si un fichier est modifiable|
|`X_OK`|Exécution autorisée|Vérifier si un CGI est exécutable|

### **Types de fichier dans un `dirent`** (`d_type`, non portable)

|Macro|Description|Utilisation dans Webserv|
|---|---|---|
|`DT_REG`|Fichier ordinaire|Affichage dans autoindex|
|`DT_DIR`|Répertoire|Affichage ou navigation|
|`DT_LNK`|Lien symbolique|Optionnel, à traiter avec soin|
