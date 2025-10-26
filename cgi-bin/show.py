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
    # Prefer PATH_TRANSLATED if provided
    translated = os.environ.get("PATH_TRANSLATED", "") or ""

    # Help if nothing specified
    if translated.strip() == "":
        print_headers()
        print("show.py - display resource from PATH_TRANSLATED or PATH_INFO")
        print("Examples:")
        print("  /cgi-bin/show.py/hello.txt  -> PATH_INFO='/hello.txt'")
        print("  PATH_TRANSLATED may be a URL like 'http://host:port/hello.txt'")
        return 0

    # If PATH_TRANSLATED looks like a URL, fetch it
    lower = translated.lower()
    if lower.startswith("http://") or lower.startswith("https://"):
        try:
            import urllib.request
            with urllib.request.urlopen(translated, timeout=5) as resp:
                data = resp.read()
            # Decode as UTF-8 text for display purposes
            content = data.decode("utf-8", errors="replace")
        except Exception as e:
            respond_error("502 Bad Gateway", f"Failed to fetch PATH_TRANSLATED: {e}")
            return 0

        print_headers()
        sys.stdout.write(content)
        return 0

    # Otherwise, treat it as a filesystem path. If relative, anchor to ./cgi-bin
    base_dir = os.path.realpath(os.path.dirname(__file__))
    target = translated
    if not os.path.isabs(target):
        target = os.path.join(base_dir, target)
    target = os.path.realpath(target)

    # Security: confine to ./cgi-bin to avoid arbitrary reads
    if not target.startswith(base_dir + os.sep) and target != base_dir:
        respond_error("403 Forbidden", "Forbidden: path traversal is not allowed")
        return 0

    if not os.path.exists(target):
        respond_error("404 Not Found", "Not Found: file does not exist")
        return 0
    if not os.path.isfile(target):
        respond_error("403 Forbidden", "Forbidden: not a regular file")
        return 0

    try:
        with open(target, "r", encoding="utf-8", errors="replace") as f:
            content = f.read()
    except Exception as e:
        respond_error("500 Internal Server Error", f"Error reading file: {e}")
        return 0

    print_headers()
    sys.stdout.write(content)
    return 0


if __name__ == "__main__":
    sys.exit(main())
