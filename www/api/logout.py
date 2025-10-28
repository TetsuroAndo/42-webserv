#!/usr/bin/python3
# -*- coding: utf-8 -*-

import os
import sys
import cgitb
import json

# add parent dir and cgi-bin dir to import storage/auth
sys.path.insert(0, os.path.dirname(os.path.dirname(__file__)))
sys.path.insert(0, os.path.join(os.path.dirname(os.path.dirname(__file__)), 'cgi-bin'))
import storage

cgitb.enable()

print("Content-Type: application/json; charset=utf-8\n\n")

session_id = os.environ.get('HTTP_X_WEBSERV_SESSION_ID')
if not session_id:
	print(json.dumps({"ok": True}))
	exit()

session_file_path = f"{storage.SESSIONS_DIR}/{session_id}.session"
try:
	os.remove(session_file_path)
	print(json.dumps({"ok": True}))
except FileNotFoundError:
	print(json.dumps({"ok": True}))
except Exception:
	print(json.dumps({"ok": False, "error": "logout_failed"}, ensure_ascii=False))
