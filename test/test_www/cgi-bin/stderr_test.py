#!/usr/bin/env python3
# CGI script that outputs to stderr for testing

import sys

# Output to stdout (normal response)
print("Content-Type: text/plain")
print("")
print("This is stdout output")

# Output to stderr (should be logged)
print("This is stderr output line 1", file=sys.stderr)
print("This is stderr output line 2", file=sys.stderr)
print("Error message: Something went wrong", file=sys.stderr)
