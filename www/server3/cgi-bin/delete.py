#!/usr/bin/env python3
import os
import sys
import urllib.parse
import cgitb

# Activer le debug CGI
cgitb.enable()

def parse_query_string():
    """ Récupère les paramètres de la requête DELETE """
    query_string = os.environ.get("QUERY_STRING", "")
    return urllib.parse.parse_qs(query_string)

def main():
    method = os.environ.get("REQUEST_METHOD", "")

    if method != "DELETE":
        print("Status: 405 Method Not Allowed")
        print("Content-Type: text/plain\n")
        print("Only DELETE method is allowed.")
        return

    params = parse_query_string()
    filename = params.get("speech", [""])[0].strip()

    if not filename:
        print("Status: 400 Bad Request")
        print("Content-Type: text/plain\n")
        print("Missing filename parameter.")
        return

    safe_filename = os.path.basename(filename)
    file_path = f"/Delete/test/{safe_filename}"

    try:
        if os.path.exists(file_path):
            os.remove(file_path)
            print("Content-Type: text/html\n")
            print(f"<h1>File '{safe_filename}' was successfully deleted 🧟‍♂️💥</h1>")
        else:
            print("Status: 404 Not Found")
            print("Content-Type: text/plain\n")
            print("File not found.")
    except Exception as e:
        print("Status: 500 Internal Server Error")
        print("Content-Type: text/plain\n")
        print(f"Error deleting file: {e}")

if __name__ == "__main__":
    main()