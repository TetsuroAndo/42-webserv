#!/usr/bin/python3
# -*- coding: utf-8 -*-

import os, sys, cgi, cgitb, json, datetime, fcntl

# add parent dir and cgi-bin dir to import storage/auth
sys.path.insert(0, os.path.dirname(os.path.dirname(__file__)))
sys.path.insert(0, os.path.join(os.path.dirname(os.path.dirname(__file__)), 'cgi-bin'))
import storage
import auth

cgitb.enable()

print("Content-Type: application/json; charset=utf-8\n\n")

# 認証チェック（未ログイン時は匿名として送信）
username = auth.get_authenticated_user() or "anonymous"

method = os.environ.get('REQUEST_METHOD')
if method != 'POST':
	print(json.dumps({"ok": False, "error": "method_not_allowed"}, ensure_ascii=False))
	exit()

form = cgi.FieldStorage()
message = form.getvalue("message")

if not message:
	print(json.dumps({"ok": False, "error": "invalid_message"}, ensure_ascii=False))
	exit()

# validate and normalize
message = message.strip()
if not message:
	print(json.dumps({"ok": False, "error": "invalid_message"}, ensure_ascii=False))
	exit()
if len(message) > 500:
	message = message[:500]
message = message.replace('\r', ' ').replace('\n', ' ')

try:
	ts = datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S')
	with open(storage.MESSAGES_LOG, 'a') as f:
		fcntl.flock(f.fileno(), fcntl.LOCK_EX)
		f.write(f"{ts}:{username}:{message}\n")
		fcntl.flock(f.fileno(), fcntl.LOCK_UN)
	print(json.dumps({"ok": True}, ensure_ascii=False))
except Exception:
	print(json.dumps({"ok": False, "error": "write_failed"}, ensure_ascii=False))
