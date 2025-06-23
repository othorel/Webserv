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
	<link href="https://fonts.googleapis.com/css2?family=Creepster&display=swap" rel="stylesheet">
	<link href="https://fonts.googleapis.com/css2?family=IM+Fell+English+SC&display=swap" rel="stylesheet">
    <style>
        body {{
			font-family: 'IM Fell English SC', sans-serif;
            background-image: url('/img/background.jpg');
            background-size: cover;
            background-position: center;
            background-repeat: no-repeat;
            text-align: center;
            padding: 40px;
            margin: 0;
            min-height: 100vh;
            color: white;
        }}
        h1 {{
            font-family: 'Creepster', cursive;
            color: darkred;
            font-size: 70px;
            text-shadow: 2px 2px 4px black, 0 10px 10px rgba(139, 0, 0, 0.6);
            margin-bottom: 20px;
			animation: fadeIn 1s ease-in-out;
        }}
        p {{
            font-size: 1.2rem;
            margin-bottom: 1rem;
        }}
		.homepage-button img{{
			width: 220px;
			border-radius: 8px;
			box-shadow: 0 4px 8px rgba(0, 0, 0, 0.2);
			transition: transform 0.3s ease, box-shadow 0.3s ease;
		}}
		.homepage-button:hover img {{
			transform: scale(1.1);
			box-shadow: 0 6px 12px rgba(0, 0, 0, 0.3);
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
    <a href="/Post/post_index.html" class="homepage-button">
		<div class="button-wrapper" style="position: relative; top:52%; left:0%; width:100%; height:100%;">
			<img src="/img/back_index.png" alt="Index" style="width: 15%; height: 15%; object-fit: cover;">
		</div>
	</a>
</body>
</html>""")
