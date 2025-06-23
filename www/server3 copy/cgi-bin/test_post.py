#!/usr/bin/env python3
import sys
import os

print("Content-Type: text/plain\n")
print("Hello from POST script!")
print("Content-Length:", os.environ.get("CONTENT_LENGTH"))
print("Request Body:", sys.stdin.read())