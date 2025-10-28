#!/usr/bin/env python3
# Echo CGI - displays POST data

import os
import sys

# Read POST data from stdin
content_length = os.environ.get('CONTENT_LENGTH', '0')
post_data = ''
if content_length and content_length != '0':
    post_data = sys.stdin.read(int(content_length))

# CGI Response Headers
print("Content-Type: text/html")
print("")

# HTML Response
print("<!DOCTYPE html>")
print("<html>")
print("<head><title>Echo CGI</title></head>")
print("<body>")
print("<h1>Echo CGI Script</h1>")
print(f"<p><strong>Request Method:</strong> {os.environ.get('REQUEST_METHOD', 'Unknown')}</p>")
print(f"<p><strong>Query String:</strong> {os.environ.get('QUERY_STRING', '(empty)')}</p>")
print(f"<p><strong>Content Type:</strong> {os.environ.get('CONTENT_TYPE', '(not set)')}</p>")
print(f"<p><strong>Content Length:</strong> {content_length}</p>")

if post_data:
    print("<h2>POST Data:</h2>")
    print(f"<pre>{post_data}</pre>")
else:
    print("<p><em>No POST data received</em></p>")

print("</body>")
print("</html>")
