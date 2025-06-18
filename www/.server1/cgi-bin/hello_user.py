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
nom = params.get("name", ["unknown"])[0]

# Récupérer la date actuelle
date_str = datetime.now().strftime("%A %d %B %Y, %H:%M:%S")

# Envoi du header CGI
sys.stdout.write("content-type: text/html\r\n")
sys.stdout.write("\r\n")

# Générer et envoyer le HTML stylisé
sys.stdout.write(f"""<!DOCTYPE html>
<html lang="fr">
<head>
    <meta charset="utf-8">
    <title>Bonjour {nom}</title>
    <style>
        body {{
            background: linear-gradient(135deg, #141e30, #243b55);
            color: #ffffff;
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            height: 100vh;
            margin: 0;
            display: flex;
            justify-content: center;
            align-items: center;
            flex-direction: column;
            text-align: center;
        }}
        h1 {{
            font-size: 2.5rem;
            color: #ffcc00;
            text-shadow: 2px 2px 6px rgba(0,0,0,0.6);
            margin-bottom: 1rem;
            animation: fadeIn 1s ease-in-out;
        }}
        p {{
            font-size: 1.2rem;
            margin-bottom: 1rem;
        }}
        a {{
            margin-top: 1rem;
            display: inline-block;
            padding: 0.6rem 1.2rem;
            background: #ffcc00;
            color: #243b55;
            font-weight: bold;
            text-decoration: none;
            border-radius: 30px;
            transition: all 0.3s ease;
            box-shadow: 0 4px 10px rgba(0, 0, 0, 0.3);
        }}
        a:hover {{
            background: #e6b800;
            transform: scale(1.05);
        }}
        @keyframes fadeIn {{
            from {{ opacity: 0; transform: translateY(-20px); }}
            to {{ opacity: 1; transform: translateY(0); }}
        }}
    </style>
</head>
<body>
    <h1>Hello, {nom} !</h1>
    <p>Current time: {date_str}.</p>
    <a href="/index.html">Retour à l'accueil</a>
</body>
</html>""")
