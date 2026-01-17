#!/usr/bin/python

import os

print("Content-Type: text/html\r\n\r\n")
print("<html><head><title>CGI Environment</title></head><body>")
print("<h1>Variables d'environnement CGI</h1>")
print("<ul>")

# Parcourt et affiche toutes les variables reçues du serveur
for key, value in os.environ.items():
    print("<li><b>{0}:</b> {1}</li>".format(key, value))

print("</ul>")
print("</body></html>")
