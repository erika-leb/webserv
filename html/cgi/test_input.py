#!/usr/bin/env python3
import sys
import os

# 1. Récupérer la taille annoncée par ton serveur C++
try:
    content_length = int(os.environ.get('CONTENT_LENGTH', 0))
except (ValueError, TypeError):
    content_length = 0

# 2. Lire le body sur stdin (le pipe de ton serveur)
# On lit exactement le nombre d'octets prévus
input_data = sys.stdin.read(content_length)

# 3. Répondre au format HTTP
print("Status: 200 OK")
print("Content-Type: text/plain")
print() # Ligne vide cruciale entre headers et body

if content_length == 0:
    print("CGI Error: Aucun contenu reçu (Content-Length = 0)")
else:
    print(f"CGI Success: Reçu {content_length} octets.")
    print(f"Contenu traité: {input_data.upper()}")
