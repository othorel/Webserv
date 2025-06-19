#!/usr/bin/env python3
import sys
from datetime import datetime

now = datetime.now().strftime("%A %d %B %Y, %H:%M:%S")

sys.stdout.write("Content-Type: text/html\r\n")
sys.stdout.write("\r\n")
sys.stdout.write(f"""<!DOCTYPE html>
<html lang="fr">
<head>
    <meta charset="utf-8">
    <title>Date actuelle</title>
    <style>
        body {{
            background: linear-gradient(135deg, #0f2027, #203a43, #2c5364);
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
            color: #00eaff;
            text-shadow: 2px 2px 6px rgba(0,0,0,0.6);
            margin-bottom: 1rem;
            animation: slideIn 1s ease-in-out;
        }}
        a {{
            margin-top: 1rem;
            display: inline-block;
            padding: 0.6rem 1.2rem;
            background: #00eaff;
            color: #003344;
            font-weight: bold;
            text-decoration: none;
            border-radius: 30px;
            transition: all 0.3s ease;
            box-shadow: 0 4px 10px rgba(0, 0, 0, 0.3);
        }}
        a:hover {{
            background: #00c7d7;
            transform: scale(1.05);
        }}
        @keyframes slideIn {{
            from {{ opacity: 0; transform: translateY(-30px); }}
            to {{ opacity: 1; transform: translateY(0); }}
        }}
    </style>
</head>
<body>
    <h1>{now}</h1>
    <a href="/index.html">Retour à l'accueil</a>
</body>
</html>""")
