#!/usr/bin/python3
# -*- coding: utf-8 -*-

import os
import sys
import cgitb
import json
import html

# add parent dir and cgi-bin dir to import storage/auth
sys.path.insert(0, os.path.dirname(os.path.dirname(__file__)))
sys.path.insert(0, os.path.join(os.path.dirname(os.path.dirname(__file__)), 'cgi-bin'))
import storage

cgitb.enable()

print("Content-Type: application/json; charset=utf-8\n\n")

limit = 50
qs = os.environ.get('QUERY_STRING') or ''
for part in qs.split('&'):
	if part.startswith('limit='):
		try:
			limit = max(1, min(200, int(part.split('=', 1)[1])))
		except Exception:
			limit = 50
		break

messages = []
try:
    with open(storage.MESSAGES_LOG, 'r') as f:
        lines = f.readlines()
        for line in lines[-limit:]:
            line = line.rstrip('\n')
            if not line:
                continue
            # 新フォーマット(JSONL)優先
            try:
                obj = json.loads(line)
                ts = str(obj.get('ts', ''))
                user = str(obj.get('user', ''))
                msg = str(obj.get('msg', ''))
                file_obj = obj.get('file') or None
                file_out = None
                if isinstance(file_obj, dict):
                    file_out = {
                        "name": html.escape(str(file_obj.get('name', ''))),
                        "url": html.escape(str(file_obj.get('url', ''))),
                        "size": int(file_obj.get('size', 0)) if str(file_obj.get('size', '')).isdigit() else 0,
                        "mime": html.escape(str(file_obj.get('mime', ''))),
                    }
            except Exception:
                # 旧フォーマット(ts:user:msg) 互換
                parts = line.split(':', 2)
                if len(parts) != 3:
                    continue
                ts, user, msg = parts
                file_out = None
            messages.append({
                "ts": html.escape(ts),
                "user": html.escape(user),
                "msg": html.escape(msg),
                "file": file_out,
            })
except FileNotFoundError:
    messages = []
except Exception:
    messages = []

print(json.dumps({"messages": messages}, ensure_ascii=False))
