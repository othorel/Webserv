
# **Le fichier de configuration** de Webserv

> Le programme Webserv doit être lancé avec en argument un chemin vers un fichier de configuration : C’est un **fichier texte**, généralement avec l’extension `.conf`, qui décrit **comment le serveur Web doit se comporter**, et qui devra être parsé à l'éxécution.

Le programme doit **lire ce fichier**, puis **configurer dynamiquement le serveur** en fonction de ce qu’il contient.

Plusieurs fichiers (donc celui par défaut) doivent être fournis avec le programme.

##### Fichier de configuration par défaut

Si le programme est lancé sans argument, il doit alors utiliser un fichier de configuration par défaut, placé à un chemin connu et codé en dur (par exemple : `"./default.conf"` ou `"/etc/webserv.conf"`).

Exemple :

```bash
./webserv config.conf     // utilise le fichier passé en argument
./webserv                  // utilise le fichier de configuration par défaut
```

## Contenu du fichier de configuration

Le fichier de configuration suit une **structure hiérarchique**. Il est composé de :

1. Un ou plusieurs blocs `server { ... }` : Chaque bloc définit un serveur virtuel. Il contient des directives et des blocs `location`
2. Un ou plusieurs blocs `location { ... }` à l’intérieur de chaque serveur : Le bloc `location` contient des directives avec les règles qui s'appliquent aux requêtes HTTP dont l'URL commence par le chemin indiqué.
3. Une **directive**, c’est une **ligne de configuration** qui donne une information au serveur sur **comment se comporter**. Elle contient le nom de la directive, et un un plusieurs arguments, séparés par des espaces. Elle se termine par `;`

#### Exemple 

```Nginx
server {
    listen 8080;
    server_name localhost;

    root /www;

    error_page 404 /404.html;

    location / {
        index index.html;
        autoindex on;
    }

    location /images {
        root /www/images;
        autoindex off;
    }

    location /cgi-bin/ {
        cgi_pass python;
    }
}
```


## Contenu du bloc `server`

### Directive `listen`

**La directive `listen` est obligatoire**, puisqu'elle définit
- le port à écouter
- et éventuellement l'adresse IP qui est **l’interface réseau** sur laquelle le serveur va écouter les connexions entrantes.

- Format attendu : `port` **ou** `IP:port`
- Exemples valides : `80`, `127.0.0.1:8080`, `0.0.0.0:3000`. On doit accepter les deux formes. Si seul le port est donné, on peut supposer que l'adresse IP est `0.0.0.0`, ce qui signifie que le serveur **écoute sur toutes les interfaces réseau de la machine**.

**Numéro de port :**
- En principe, n'importe quel port compris entre `1` et `65535`**, tant qu'il est valide, disponible et adapté à tl'usage**.
- Le **port `80`** est le **port standard pour le protocole HTTP**.
- **Mais ur un système Unix/Linux/Mac**, les ports **< 1024** sont **réservés aux processus root**.
    - Un utilisateur normal (ce qui est le cas à 42), ne pourra pas utiliser le port 80 sans sudo.
    - Donc pour tester, **utiliser plutôt un port ≥ 1024** (par exemple `8080`, `8000`, `4242`, etc.).

**Adresse IP :**
- Il s’agit de **l’interface réseau** sur laquelle le serveur va écouter les connexions entrantes.
- `127.0.0.1` → localhost (accessibles uniquement depuis la machine locale)
- `0.0.0.0` → toutes les interfaces (Wi-Fi, Ethernet, VPN, etc.)
- `192.168.x.x` ou `10.x.x.x` → réseau local (ex : machine dans le réseau Wi-Fi)
- Adresse publique

### Directive `server_name` 

La directive `server_name` est optionnelle, mais elle permet de :
- **distinguer plusieurs serveurs virtuels** (`server { ... }`) qui écoutent sur le **même couple IP:port**
- sélectionner le bon bloc `server` en fonction de l'en-tête `Host:` reçu dans la requête HTTP.

- **Format attendu :** une ou plusieurs chaînes (noms de domaine ou alias), séparées par des espaces
- **Exemples valides :** `localhost`, `www.example.com`, `localhost site1.com`

Serveurs virtuels :
- Lorsque plusieurs blocs `server` sont définis, on parle alors de serveurs virtuels (*virtual hosts*)
- Un **serveur virtuel** est une configuration distincte du serveur web, qui peut **réagir différemment selon l'adresse IP, le port, ou le nom de domaine (Host)** utilisé dans la requête HTTP.

**Cas d'usage typique :**
- Plusieurs blocs `server` écoutent sur la **même IP et le même port**
- Le serveur sélectionne le bon bloc à utiliser en **comparant la valeur de l'en-tête `Host:`** à la valeur de `server_name`

**Comportement sans `server_name` :**
- Le bloc `server` sans `server_name` devient le **serveur par défaut**
- Il est utilisé **si aucun autre `server` ne correspond au nom d’hôte reçu**
- S'il y a plusieurs blocs `server` sans `server_name`, c'est le premier qui sera utilisé comme **serveur par défaut**

**Remarques :**
- Il n’est **pas nécessaire d’utiliser de guillemets** autour des noms.
- Le nom comparé est sensible à la **casse et au format exact** (pas d’espaces, pas de slashs).
- On peut définir **plusieurs noms** pour un même serveur (`server_name site.com www.site.com localhost;`).

### Directive `root`

**La directive `root` est obligatoire**, car elle donne le chemin vers un répertoire sur le système de fichiers du serveur, qui est utilisé comme **point de départ** (racine) pour rechercher les fichiers demandés dans les requêtes HTTP.

- Format attendu : chemin absolu ou relatif vers un dossier
- Exemples valides : `/var/www/html`, `./www/`, `html/`

Elle peut apparaître à deux niveaux :
- dans un bloc `server` : elle s’applique alors à toutes les requêtes du serveur, sauf si surchargée
- dans un bloc `location` : elle **surchage** la `root` définie au niveau `server` pour ce chemin spécifique : **la directive `root` définie dans un bloc `location` prend le dessus sur celle définie dans le bloc `server`**, **pour toutes les requêtes qui matchent cette `location`**

**Comportement attendu :**
Lorsqu’une requête est reçue (par exemple `GET /index.html`), le serveur concatène la `root` et le chemin demandé :
- Si `root /var/www/html;` et requête `/index.html` → serveur cherche `/var/www/html/index.html`
- Si `location /images` a `root /var/www/img;` → requête `/images/logo.png` donnera `/var/www/img/logo.png`

**Remarques :**
- La `root` ne doit **pas se terminer par un slash `/`**, car le chemin de l’URL est ajouté directement après.
- Si la `root` n’existe pas ou est inaccessible, le serveur devra retourner une **erreur 403 ou 404**.
- Le chemin peut être **absolu** (`/home/user/www`) ou **relatif** au dossier de lancement de Webserv (`./html`)

**Bonnes pratiques :**
- Centraliser les fichiers statiques (HTML, CSS, JS, images…) dans des répertoires bien organisés
- Définir une `root` par `location` uniquement si nécessaire, pour éviter les confusions

### Directive `error_page`

**La directive `error_page` est obligatoire** dans Webserv, car elle permet de définir **des pages personnalisées** à afficher en cas d’erreur HTTP (au minimum pour le code 404, selon le sujet).

- Format attendu : `code /chemin/vers/page.html;`
- Exemples valides : `error_page 404 /404.html;`, `error_page 500 /500.html;`

**Elle peut apparaître :**
- au niveau du bloc `server` : s’applique à toutes les requêtes traitées par ce serveur
- ou dans un bloc `location` : s’applique uniquement aux erreurs rencontrées dans ce contexte spécifique

**Comportement attendu :**
- Quand une erreur survient (ex : fichier non trouvé → 404), le serveur redirige la réponse vers la page définie.
- Le chemin fourni (`/404.html`) est un chemin relatif à la `root` applicable.
    - Par exemple, si `root /var/www/html;` et `error_page 404 /404.html;`  
        → Le serveur cherche le fichier `/var/www/html/404.html`.

**Plusieurs codes peuvent être traités :**
- On peut avoir autant de directives `error_page` que de codes à gérer.
- Les codes fréquents sont : `400`, `403`, `404`, `405`, `413`, `500`, `502`, `504`

**Remarques :**
- L’URL donnée dans `error_page` **doit commencer par `/`** (URI côté client, pas un chemin système direct).
- Le fichier pointé doit exister et être accessible, sinon on risque une **cascade d’erreurs** (ex : 404 qui mène à une autre 404).
- Le serveur **ne modifie pas le code HTTP** : il retourne le **même code**, mais avec le **contenu personnalisé**.

### Bloc `location`

Le bloc `location` est obligatoire**, car c’est lui qui permet de définir **les règles spécifiques à appliquer pour un chemin donné de l’URL** (URI).
Un bloc `location` est **imbriqué dans un bloc `server`** et délimité par des accolades `{ ... }`.

- **Format attendu :** `location /path { ... }`
- **Exemples valides :**
    - `location / { ... }` → racine, s’applique à toutes les requêtes
    - `location /images { ... }` → s’applique aux URL qui commencent par `/images`
    - `location /cgi-bin/ { ... }` → souvent utilisé pour déclencher un script CGI

**Comportement attendu :**
- Le serveur choisit le bloc `location` **qui correspond le mieux** au début de l’URI demandée.
- Le bloc peut ensuite définir des **règles spécifiques** pour ce chemin : `methods`, `root`, `index`, `autoindex`, `cgi_pass`, etc.
- Si **plusieurs `location` matchent**, le **plus long préfixe commun** avec l’URI est prioritaire (_prefix matching_).

**Remarques :**
- Tu peux avoir plusieurs blocs `location` par `server`.
- S’il n’y a pas de bloc `location` qui matche, le comportement est généralement une erreur 404.
- Le bloc `location /` est souvent utilisé comme **fallback général** (c'est ce bloc qui sera utilisé si aucune autre `locatopn` ne correspond à la requête).

## Contenu du bloc `server`
### Directive `methods`

**La directive `methods` est obligatoire dans chaque bloc `location`**, car elle définit **quelles méthodes HTTP** sont **autorisées** pour les requêtes correspondant à cette `location`.

- **Format attendu :** une ou plusieurs méthodes parmi `GET`, `POST`, `DELETE`, séparées par des espaces et terminées par un `;`
- **Exemples valides :**
    - `methods GET;`
    - `methods GET POST;`
    - `methods GET POST DELETE;`

**Méthodes à gérer obligatoirement dans Webserv :**
- `GET` : pour récupérer une ressource (page HTML, image…)
- `POST` : pour envoyer des données (formulaires, upload…)
- `DELETE` : pour supprimer une ressource (rarement utilisé côté client, mais exigé dans le sujet)

**Comportement attendu :**
- Si une requête utilise une méthode **non autorisée** dans la `location` ciblée, le serveur doit renvoyer une **erreur 405 Method Not Allowed**, avec un en-tête `Allow:` listant les méthodes valides.

**Remarques :**

- L’ordre des méthodes n’a pas d’importance.
- Si aucune méthode n’est précisée (erreur de config), le comportement du serveur est **non défini**, mais dans Webserv, cela doit déclencher une **erreur de parsing**.
- Il faut valider les noms : toute méthode autre que `GET`, `POST`, `DELETE` est à **rejeter explicitement**.

### Directive `index`

**La directive `index` est obligatoire dans chaque bloc `location`**, car elle permet de définir **le ou les fichiers à renvoyer** lorsqu’une requête vise un dossier (URI se terminant par `/`).

- Format attendu : un ou plusieurs noms de fichiers, séparés par des espaces et terminés par `;`
- Exemples valides :
    - `index index.html;`
    - `index index.html index.htm;`

**Comportement attendu :**
- Si la requête cible une URI de type répertoire (`/` ou `/dossier/`), le serveur essaie de retourner l’un des fichiers listés dans `index`, dans l’ordre donné.
- Par exemple :
    - requête : `GET /blog/`
    - avec `index index.html index.htm;`
    - alors Webserv cherche (dans le dossier `root` approprié) :
        - `/blog/index.html`
        - puis `/blog/index.htm`
    - Si aucun fichier n’existe, deux cas :
        - si `autoindex on;` → le serveur génère un listing du dossier
        - sinon → erreur `403 Forbidden` ou `404 Not Found`

**Remarques :**
- Le ou les fichiers `index` sont relatifs au `root` défini pour cette `location`.
- Il est possible de spécifier plusieurs fichiers (fallbacks).
- Ne pas confondre `index` (fichier de bienvenue) avec `root` (chemin de base).

### Directive `autoindex`

**La directive `autoindex` est obligatoire dans chaque bloc `location`**, car elle contrôle si le serveur doit ou non **générer automatiquement un listing du contenu d’un dossier** lorsque :
- une requête vise un répertoire,
- et qu’**aucun fichier `index` n’est présent**.

- **Format attendu :** `autoindex on;` ou `autoindex off;`
- **Exemples valides :**
    - `autoindex on;`
    - `autoindex off;`

**Comportement attendu :**
- `autoindex on;` : si l’URI cible un dossier **et** qu’aucun fichier `index` n’est trouvé, Webserv renvoie une **page HTML générée dynamiquement** listant le contenu du dossier.
- `autoindex off;` : dans le même cas, le serveur doit renvoyer une **erreur 403 Forbidden** ou **404 Not Found** (selon ton implémentation).

**Exemple pratique :**

```conf
location /images {
    root /var/www/assets;
    index index.html;
    autoindex on;
}
```

→ Si `/var/www/assets/index.html` n'existe pas, et qu’on fait une requête vers `/images/`,  
Webserv génère une page HTML listant les fichiers de `/var/www/assets/`.

**Remarques :**
- Le sujet Webserv **exige que le listing autoindex soit une vraie page HTML**, pas juste du texte.
- Le fichier généré dynamiquement doit être correctement encodé (UTF-8) et lisible dans un navigateur.
- Ne pas confondre `autoindex` (listing automatique) avec `index` (fichier à afficher par défaut).
Parfait ! Voici la fiche pour la directive `cgi_pass`, qui clôture les directives du bloc `location` :

### Directive `cgi_pass`

**La directive `cgi_pass` est obligatoire dans un bloc `location` uniquement si ce chemin est destiné à exécuter des scripts CGI** (par exemple `.py`, `.php`, etc.).  
Elle indique **le chemin absolu de l’interpréteur** utilisé pour exécuter les scripts CGI correspondants.

- **Format attendu :** `cgi_pass /chemin/vers/interpréteur;`
- **Exemples valides :**
    - `cgi_pass /usr/bin/python3;`
    - `cgi_pass /usr/bin/php;`

**Comportement attendu :**
- Si une requête cible un fichier géré par CGI (ex : `/cgi-bin/script.py`), le serveur :
    1. Utilise l’interpréteur défini par `cgi_pass`
    2. Lance un processus enfant via `fork()` + `execve()`
    3. Passe les données via des pipes (entrée standard, sortie standard)
    4. Récupère la sortie du script et la renvoie au client comme réponse HTTP

**Conditions requises :**
- Le fichier ciblé doit exister, être lisible et **exécutable**.
- Le chemin `cgi_pass` doit pointer vers un **exécutable valide**.
- On doit construire correctement les **variables d’environnement CGI** (comme `REQUEST_METHOD`, `CONTENT_LENGTH`, etc.)

**Remarques :**
- Le bloc `location` contenant un `cgi_pass` est souvent spécifique à un répertoire (`/cgi-bin/`) ou à une extension (`.py`, `.php`)
- On peut choisir de ne déclencher le CGI que **pour certains fichiers uniquement**, par convention ou en ajoutant un filtre dans le parser.

**Exemple d'utilisation :**

```conf
location /cgi-bin/ {
    root /var/www/cgi-bin;
    methods GET POST;
    autoindex off;
    cgi_pass /usr/bin/python3;
}
```

## Validations à faire au parsing du fichier de configuration

|Champ|Vérification à faire|
|---|---|
|`listen`|Le port doit être un entier entre 1 et 65535. L’IP si présente doit être valide (`x.x.x.x`).|
|`server_name`|Ne doit pas être vide. Peut contenir plusieurs noms.|
|`root`|Doit être un chemin accessible, tu peux tester avec `access(root.c_str(), F_OK)`|
|`error_page`|Le code doit être un entier HTTP connu (au moins 404). Le chemin doit être relatif à `root`.|
|`methods`|Uniquement `GET`, `POST`, `DELETE`. Refuser autre chose.|
|`index`|Fichier(s) à chercher quand une URI finit par `/`.|
|`autoindex`|Doit être `on` ou `off`, sinon erreur de parsing.|
|`cgi_pass`|Doit être un chemin exécutable (`access(..., X_OK)` possible).|

