#!/usr/bin/env python3
print("Status: 404 Not Found")
print("Content-Type: text/plain")
print("")
print("This CGI exits with status 1 but has Status header set to 404")
exit(1)
