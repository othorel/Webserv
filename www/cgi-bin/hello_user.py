#!/usr/bin/env python3
import os
import sys
from datetime import datetime
import urllib.parse

# Lire la taille du body (envoyé par POST)
content_length = int(os.environ.get("CONTENT_LENGTH", 0))
post_data = sys.stdin.read(content_length)

# Parser le body (ex: nom=Lucas)
params = urllib.parse.parse_qs(post_data)
nom = params.get("nom", ["inconnu"])[0]

# Récupérer la date actuelle
date_str = datetime.now().strftime("%A %d %B %Y, %H:%M:%S")

# Générer la page HTML
print("Content-Type: text/html")
print()
print(f"""<!DOCTYPE html>
<html>
<head>
    <title>Bonjour {nom}</title>
    <meta charset="utf-8">
</head>
<body>
    <h1>Hello, {nom} !</h1>
    <p>Nous sommes le {date_str}.</p>
</body>
</html>""")
