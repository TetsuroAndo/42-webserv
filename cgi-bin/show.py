#!/usr/bin/env python3
"""
CGI script: show the contents of a file specified via PATH_INFO.

Usage examples:
  - /cgi-bin/show.py/hello.txt            -> shows ./cgi-bin/hello.txt
  - /cgi-bin/show.py/subdir/file.md       -> shows ./cgi-bin/subdir/file.md

Security:
  - Resolves the requested path under this script's directory (./cgi-bin)
  - Blocks path traversal attempts and non-file targets
"""

import os
import sys


def print_headers(status: str = "200 OK", content_type: str = "text/plain; charset=utf-8") -> None:
    print(f"Status: {status}")
    print(f"Content-Type: {content_type}")
    print("X-Content-Type-Options: nosniff")
    print("")


def respond_error(status: str, message: str) -> None:
    print_headers(status=status)
    print(message)


def main() -> int:
    path_info = os.environ.get("PATH_INFO", "") or ""

    # Expect PATH_INFO like "/relative/path"
    # Normalize and ensure it's confined to ./cgi-bin
    rel_path = os.path.normpath(path_info.lstrip("/"))

    base_dir = os.path.realpath(os.path.dirname(__file__))
    target = os.path.realpath(os.path.join(base_dir, rel_path))

    # Security checks: must be within base_dir
    if not target.startswith(base_dir + os.sep) and target != base_dir:
        respond_error("403 Forbidden", "Forbidden: path traversal is not allowed")
        return 0

    # If no file specified, show brief help
    if rel_path in ("", "."):
        print_headers()
        print("show.py - display a file under ./cgi-bin via PATH_INFO")
        print("Usage: /cgi-bin/show.py/<relative/path>")
        return 0

    if not os.path.exists(target):
        respond_error("404 Not Found", "Not Found: file does not exist")
        return 0

    if not os.path.isfile(target):
        respond_error("403 Forbidden", "Forbidden: not a regular file")
        return 0

    try:
        # Read as text; replace undecodable bytes to avoid 500s
        with open(target, "r", encoding="utf-8", errors="replace") as f:
            content = f.read()
    except Exception as e:
        respond_error("500 Internal Server Error", f"Error reading file: {e}")
        return 0

    print_headers()
    # Output file content as plain text
    sys.stdout.write(content)
    return 0


if __name__ == "__main__":
    sys.exit(main())

