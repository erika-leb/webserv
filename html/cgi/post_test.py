#!/usr/bin/python

import sys
import os

# Lire le body envoyé par le serveur via stdin
content_length = int(os.environ.get('CONTENT_LENGTH', 0))
body = sys.stdin.read(content_length)

print("Content-Type: text/plain\r\n\r\n")
print("--- Resultat CGI ---")
print("Methode: {0}".format(os.environ.get('REQUEST_METHOD')))
print("Taille du body: {0}".format(content_length))
print("Contenu du body: {0}".format(body))
