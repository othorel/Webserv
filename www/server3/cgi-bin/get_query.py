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
        a {{
            display: inline-block;
            font-size: 20px;
            padding: 10px 20px;
            background-color: #8da6ec;
            color: black;
            text-decoration: none;
            border-radius: 5px;
            transition: background-color 0.3s ease, transform 0.3s ease;
            text-shadow: 1px 1px 2px black;
        }}
        a:hover {{
            background-color: #0300b2;
            transform: scale(1.1) rotate(3deg);
            color: white;
        }}
    </style>
</head>
<body>
    <h1>GET Parameters Received</h1>
    <p><strong>Name:</strong> {name}</p>
    <p><strong>Age:</strong> {age}</p>
    <a href="/Get/page5.html">⬅ Back to Form</a>
</body>
</html>
""")

except Exception as e:
    print("Content-Type: text/plain\r\n\r\n")
    print("CGI Error:", file=sys.stderr)
    print(e)
