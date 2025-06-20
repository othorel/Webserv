#!/usr/bin/env python3
import cgi
import cgitb
import os
import sys

# Enable CGI debugging (shows detailed error page in browser)
cgitb.enable()

try:
    # Get query string parameters
    form = cgi.FieldStorage()

    # Extract values safely
    name = form.getfirst("name", "Unknown").strip()
    age = form.getfirst("age", "Not specified").strip()

    # Send HTTP header
    print("Content-Type: text/html\r\n\r\n")

    # Generate HTML output
    print(f"""<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <title>CGI GET - Parameters</title>
    <style>
        body {{
            background-color: #222;
            color: #fff;
            font-family: Arial, sans-serif;
            padding: 40px;
            text-align: center;
        }}
        h1 {{
            color: #00eaff;
        }}
        p {{
            font-size: 20px;
        }}
        a {{
            color: #00eaff;
            text-decoration: none;
            font-weight: bold;
        }}
        a:hover {{
            text-decoration: underline;
        }}
    </style>
</head>
<body>
    <h1>Received Parameters</h1>
    <p><strong>Name:</strong> {name}</p>
    <p><strong>Age:</strong> {age}</p>
    <p><a href="/Get/page5.html">⬅ Back</a></p>
</body>
</html>
""")
except Exception as e:
    print("Content-Type: text/plain\r\n\r\n")
    print(f"CGI Error: {e}", file=sys.stderr)
