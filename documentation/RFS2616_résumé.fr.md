# Résumé pédagogique du RFC 2616

## 1. Introduction

HTTP est un protocole de communication **client/serveur** conçu pour le Web.  
Il est **sans état** (le serveur ne se souvient pas des requêtes précédentes) et fonctionne principalement en **mode texte** sur TCP.

HTTP/1.1 améliore HTTP/1.0 par :

- la **persistance des connexions**
    
- une meilleure **gestion du cache**
    
- le support explicite des **hôtes virtuels** (via l’en-tête `Host`)
    

Il pose les **fondations du web moderne**.

---

## 2. Notation et grammaire

Cette section formalise la **syntaxe du protocole**.

Elle explique :

- les mots-clés comme `MUST`, `SHOULD`, `MAY` (niveaux d’obligation)
    
- la grammaire utilisée (ABNF) pour définir les règles de syntaxe
    
- les notions de `token`, `quoted-string`, et l’usage du charset ASCII
    

> Ces bases sont utilisées dans toutes les sections suivantes pour **exprimer les formats de message de manière précise**.

---

## 3. Paramètres du protocole

Définit les éléments **transversaux** du protocole :

- **Version HTTP** (ex: `HTTP/1.1`)
    
- **URI** pour identifier les ressources
    
- **Dates** en format RFC 1123
    
- **Types MIME**, langues, encodages (`gzip`, `UTF-8`)
    
- Gestion des préférences avec `q=0.9`
    

> Ces paramètres permettent au client et au serveur de **communiquer leurs capacités et préférences**.

---

## 4. Messages HTTP

Décrit la **structure de base des échanges** :

- **Requêtes** (Request-Line + headers + corps)
    
- **Réponses** (Status-Line + headers + corps)
    
- En-têtes et corps sont séparés par une ligne vide
    
- Le corps est optionnel (selon la méthode ou le statut)
    

> Chaque message HTTP est structuré avec rigueur, ce qui permet l’interopérabilité entre clients et serveurs.

---

## 5. Requêtes

Explique comment un client **formule une requête**.

- Ligne de requête : `METHOD URI VERSION`
    
- Méthodes standard : `GET`, `POST`, `PUT`, `DELETE`, etc.
    
- URI peut être relative ou absolue
    
- En-tête `Host` est obligatoire en HTTP/1.1
    

> Le client **demande une ressource** au serveur à l’aide d’une requête bien structurée.

---

## 6. Réponses

Explique comment le serveur **répond** :

- Ligne de statut : `HTTP-Version Code Raison`
    
- Classes de codes :
    
    - `2xx` succès
        
    - `3xx` redirection
        
    - `4xx` erreur client
        
    - `5xx` erreur serveur
        
- Le corps est facultatif selon le code (ex: 204 No Content)
    

> Le serveur **indique l’issue de la requête** avec un code standardisé.

---

## 7. Entités

Une **entité** = contenu + métadonnées.

- Le **corps** de l’entité contient les données (HTML, image…)
    
- Les **en-têtes d’entité** décrivent ce contenu (`Content-Type`, `Length`, `Encoding`, `Language`, etc.)
    

> Les entités transportent **les vraies données** échangées entre client et serveur.

---

## 8. Connexions

HTTP/1.1 **réutilise les connexions TCP** par défaut (keep-alive).

- Connexions persistantes → meilleures performances
    
- Le client peut envoyer plusieurs requêtes d’affilée (pipelining)
    
- En-tête `Connection: close` pour forcer la fermeture
    
- Mécanisme `Expect: 100-continue` pour optimiser les uploads
    

> Cette section explique **comment HTTP gère les connexions sous-jacentes**, de manière efficace.

---

## 9. Définitions des méthodes

Détaille le comportement attendu de chaque **méthode HTTP** :

- `GET` : récupérer
    
- `HEAD` : comme GET, mais sans le corps
    
- `POST` : envoyer des données à traiter
    
- `PUT` : remplacer ou créer
    
- `DELETE` : supprimer
    
- `OPTIONS`, `TRACE`, `CONNECT` : usages plus techniques
    

> Chaque méthode a une **sémantique propre** à respecter côté client et serveur.

---

## 10. Codes de statut

Explique tous les **codes de réponse HTTP**, répartis par famille :

- `100` → traitement en cours
    
- `200` → succès
    
- `300` → redirections
    
- `400` → erreurs côté client
    
- `500` → erreurs côté serveur
    

> Les codes permettent à chaque partie de **comprendre l’état de l’échange**.

---

## 11. Authentification

Décrit le mécanisme d’**accès protégé** :

- Le serveur répond avec `401 Unauthorized` + `WWW-Authenticate`
    
- Le client envoie `Authorization`
    
- Support de `Basic` et `Digest` (non sécurisé sans HTTPS)
    
- Authentification proxy : `407`, `Proxy-Authenticate`, `Proxy-Authorization`
    

> HTTP prévoit une **authentification simple mais extensible**.

---

## 12. Négociation de contenu

Permet au client de spécifier ses **préférences** :

- Type (`Accept`), langue (`Accept-Language`), encodage, charset
    
- Le serveur choisit la représentation la plus adaptée
    
- Possibilité de négociation côté client (`300 Multiple Choices`)
    
- Impact sur le cache : en-tête `Vary`
    

> Une même ressource peut avoir **plusieurs représentations selon le contexte**.

---

## 13. Cache HTTP

Définit les règles de **mise en cache** des réponses :

- En-têtes : `Cache-Control`, `Expires`, `ETag`, `Last-Modified`
    
- Contrôle fin avec `no-store`, `max-age`, `private`, `must-revalidate`
    
- Validation via `If-Modified-Since`, `If-None-Match`
    
- Prévention des fuites via `Vary`, `private`, etc.
    

> Un bon usage du cache permet **rapidité, économie de ressources et cohérence**.

---

## 14. Champs d’en-tête

Dresse la liste **complète et normée** de tous les en-têtes HTTP/1.1 :

- `Content-Type`, `User-Agent`, `Host`, `Accept`, `Authorization`, etc.
    
- En-têtes conditionnels, de cache, de sécurité…
    
- Description du rôle, de la syntaxe, et du comportement attendu
    

> Ces en-têtes sont **les fondations sémantiques du protocole HTTP**.

---

## 15. Sécurité

Identifie les **risques liés à HTTP** :

- Absence de chiffrement → utiliser HTTPS
    
- Authentification vulnérable en clair
    
- Risques de falsification, injection d’en-têtes, redirection malveillante
    
- Problèmes liés au cache, au contenu actif (scripts), à la confidentialité
    

> HTTP/1.1 nécessite **des mesures complémentaires** pour sécuriser les échanges (comme TLS, validations, tokens, etc.).
