#!/usr/bin/env python3

import cgi

print("Content-Type: text/html")
print()  # End headers

print("""
<!DOCTYPE html>
<html>
<head>
    <title>Simple Python CGI</title>
</head>
<body>
    <h1>Hello from Python CGI!</h1>
    <p>This is a basic CGI script running on your server.</p>
</body>
</html>
""")

# while True:
# 	print("n;importe quoi")