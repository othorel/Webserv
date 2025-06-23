#!/usr/bin/env python3
import cgi
import cgitb
import os
import sys

# Enable CGI error debugging
cgitb.enable()

try:
    # Get query parameters
    form = cgi.FieldStorage()
    name = form.getfirst("name", "Unknown").strip()
    age = form.getfirst("age", "Not specified").strip()
    # while(True):
    #     print("boucle infinie ")

    # Send HTTP header
    print("Content-Type: text/html\r\n\r\n")

    # Generate HTML response
    print(f"""<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <title>Zombie CGI - GET Parameters</title>
    <link href="https://fonts.googleapis.com/css2?family=Creepster&display=swap" rel="stylesheet">
    <style>
        body {{
            font-family: Arial, sans-serif;
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
        }}
        p {{
            font-size: 22px;
            margin-bottom: 20px;
            text-shadow: 1px 1px 2px black;
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
    </style>
</head>
<body>
    <h1>GET Parameters Received</h1>
    <p><strong>Name:</strong> {name}</p>
    <p><strong>Age:</strong> {age}</p>
    <a href="/Get/get_index.html" class="homepage-button">
		<div class="button-wrapper" style="position: relative; top:52%; left:0%; width:100%; height:100%;">
			<img src="/img/back_index.png" alt="Index" style="width: 15%; height: 15%; object-fit: cover;">
		</div>
	</a>
</body>
</html>
""")

except Exception as e:
    print("Content-Type: text/plain\r\n\r\n")
    print("CGI Error:", file=sys.stderr)
    print(e)
