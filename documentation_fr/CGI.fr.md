
# CGI (Common Gateway Interface)

> Le **CGI** est une **interface standard** qui permet à un serveur web d’**exécuter un programme externe** (souvent un script, comme Python ou PHP), et d’en **utiliser la sortie comme réponse HTTP**.

## Généralités
### Objectif du CGI

Permettre au serveur de générer des réponses **dynamiques**, en exécutant un programme à chaque requête HTTP ciblant un script (ex: `.py`, `.php`, `.cgi`, etc.).

>Contrairement aux fichiers HTML statiques, la réponse **n'est pas lue depuis un fichier**, mais **générée à la volée** par un processus externe.

Par exemple, on peut utiliser un script CGI pour : un formulaire de contact, une page de connexion (login), un système de commentaire, la génération d'un graphique à la volée, une calculatrice en ligne, etc.
### Fonctionnement global dans Webserv

1. Le client envoie une requête vers un chemin géré par un CGI (ex: `/cgi-bin/script.py`).
2. Webserv détecte que ce fichier est un script CGI (via `location` + `cgi_pass`).
3. Webserv crée deux pipes :
    - un pour **envoyer des données** au script (stdin)
    - un pour **recevoir la sortie** du script (stdout)
4. Webserv appelle `fork()`, puis `execve()` dans le processus enfant :
    - le script est exécuté avec les **bonnes variables d’environnement**
    - la sortie du script est capturée
5. Le parent lit la sortie, l’analyse (en-têtes + corps) et la renvoie au client.

### Structure typique d’une configuration Webserv avec CGI

```conf
location /cgi-bin/ {
    root /var/www/cgi-bin;
    methods GET POST;
    autoindex off;
    cgi_pass /usr/bin/python3;
}
```

- Les requêtes vers `/cgi-bin/...` sont gérées par le CGI
- Le script est exécuté via `/usr/bin/python3`    
- Le fichier cible doit être accessible et exécutable

---
## Implémentation dans Webserv

### 1. **Détection du CGI**

- Le parser doit associer le `path` (ou son extension `.py`, `.php`, etc.) à un bloc `location` contenant `cgi_pass`.
- Le `cgi_pass` contient le chemin vers l’interpréteur (ex: `/usr/bin/python3`).

### 2. **Création des pipes**

- Un pipe pour écrire (le POST ou stdin)
- Un pipe pour lire (stdout du script)    

### 3. **Préparation du `execve()`**

```cpp
char *argv[] = { "python3", "script.py", NULL };
char *envp[] = { "REQUEST_METHOD=GET", ..., NULL };

execve("/usr/bin/python3", argv, envp);
```

- `argv[0]` est l’interpréteur
- `argv[1]` est le chemin du script
- `envp` contient les **variables CGI obligatoires** (voir ci-dessous)

### 4. **Variables d’environnement (CGI standard)**

|Variable CGI|Description|
|---|---|
|`REQUEST_METHOD`|Méthode HTTP utilisée (`GET`, `POST`, etc.)|
|`SCRIPT_NAME`|Nom du script|
|`QUERY_STRING`|Partie après `?` dans l’URL|
|`CONTENT_LENGTH`|Taille du corps (pour POST)|
|`CONTENT_TYPE`|Type du corps (ex: `application/x-www-form-urlencoded`)|
|`SERVER_PROTOCOL`|Version HTTP (`HTTP/1.1`)|
|`GATEWAY_INTERFACE`|Toujours `CGI/1.1`|
|`SERVER_SOFTWARE`|Ex: `Webserv/1.0`|
|`SERVER_NAME`|Nom du serveur (dérivé du Host)|
|`SERVER_PORT`|Port utilisé|
|`REMOTE_ADDR`|Adresse IP du client|

### 5. **Lecture de la sortie du CGI**

- Le script CGI **doit retourner un contenu HTTP complet** :
    - au moins un en-tête `Content-Type: text/html`
    - une ligne vide (`\r\n`) séparant les en-têtes du corps
- Webserv lit cette sortie, la découpe (headers + body) et la renvoie comme réponse HTTP.

---
## Exemple de script CGI Python

```python
#!/usr/bin/env python3
print("Content-Type: text/html")
print()
print("<html><body><h1>Hello from CGI</h1></body></html>")
```

⚠️ Aucun espace avant `print()`, et il faut bien **deux sauts de ligne** entre les en-têtes et le corps.

---
## Sécurité et contraintes

- Le script doit être **exécutable** : `chmod +x script.py`
- Ne jamais exécuter un script non contrôlé (pas de `eval`, pas de shell)
- Vérifier que le chemin cible est bien sous le `root` de la `location`
- Gérer les erreurs si :
    - le script échoue
    - ne renvoie rien
    - envoie un code HTTP invalide
- Timeout recommandé pour éviter les scripts bloquants

---
##  A implémenter dans Webserv

|Étape|Détails|
|---|---|
|Détection du chemin CGI|Par extension, ou `location` dédiée|
|Construction de l'environnement|Via variables CGI|
|Fork + pipe + execve|Gestion classique d’un processus|
|Lecture + parsing de la sortie|Split headers/body, gérer status code|
|Envoi de la réponse HTTP|Headers + body vers le client|

