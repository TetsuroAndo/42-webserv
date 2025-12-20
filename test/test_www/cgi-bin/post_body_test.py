#!/usr/bin/env python3
# CGI script that receives POST body for testing request body logging

import os
import sys

# Read POST data from stdin
content_length = os.environ.get('CONTENT_LENGTH', '0')
post_data = ''
if content_length and content_length != '0':
    post_data = sys.stdin.read(int(content_length))

# CGI Response Headers
print("Content-Type: text/plain")
print("")

# Echo back the POST data
print(f"Received POST data: {post_data}")
