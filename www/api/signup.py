#!/usr/bin/python3
# -*- coding: utf-8 -*-

import os
import sys
import cgi
import cgitb
import hashlib
import json
import re

# add parent dir and cgi-bin dir to import storage/auth
sys.path.insert(0, os.path.dirname(os.path.dirname(__file__)))
sys.path.insert(0, os.path.join(os.path.dirname(os.path.dirname(__file__)), 'cgi-bin'))
import storage

cgitb.enable()

print("Content-Type: application/json; charset=utf-8\n\n")

method = os.environ.get('REQUEST_METHOD')
if method != 'POST':
	print(json.dumps({"ok": False, "error": "method_not_allowed"}, ensure_ascii=False))
	exit()

form = cgi.FieldStorage()
username = form.getvalue("username")
password = form.getvalue("password")

if not username or not password:
	print(json.dumps({"ok": False, "error": "missing_credentials"}, ensure_ascii=False))
	exit()

username = username.strip()
if not re.fullmatch(r"[A-Za-z0-9_]{3,20}", username or ""):
	print(json.dumps({"ok": False, "error": "invalid_username"}, ensure_ascii=False))
	exit()
if len(password) < 6 or len(password) > 128:
	print(json.dumps({"ok": False, "error": "invalid_password"}, ensure_ascii=False))
	exit()

hashed_password = hashlib.sha256(password.encode()).hexdigest()

# 重複チェック
try:
	with open(storage.USERS_DB, 'r') as f:
		for line in f:
			if line.startswith(f"{username}:"):
				print(json.dumps({"ok": False, "error": "user_exists"}, ensure_ascii=False))
				exit()
except FileNotFoundError:
	pass

# 追記
try:
	os.makedirs(os.path.dirname(storage.USERS_DB), exist_ok=True)
	with open(storage.USERS_DB, 'a') as f:
		f.write(f"{username}:{hashed_password}\n")
	print(json.dumps({"ok": True}, ensure_ascii=False))
except Exception:
	print(json.dumps({"ok": False, "error": "write_failed"}, ensure_ascii=False))
