#!/usr/bin/python3
# -*- coding: utf-8 -*-

import os, cgitb, json, html
import auth, storage

cgitb.enable()

def respond(status_code=200, payload=None):
    if status_code != 200:
        print(f"Status: {status_code} Unauthorized" if status_code == 401 else f"Status: {status_code}")
    print("Content-Type: application/json; charset=utf-8\n\n")
    print(json.dumps(payload if payload is not None else {}))

# 認証チェック
username = auth.get_authenticated_user()
if not username:
    respond(401, {"error": "unauthorized"})
    raise SystemExit

# クエリ: limit
query = os.environ.get('QUERY_STRING', '')
limit = 100
try:
    for pair in query.split('&'):
        if not pair:
            continue
        k, _, v = pair.partition('=')
        if k == 'limit' and v.isdigit():
            limit = max(1, min(500, int(v)))
            break
except Exception:
    pass

messages = []
try:
    with open(storage.MESSAGES_LOG, 'r') as f:
        for line in f:
            parts = line.strip().split(':', 2)
            if len(parts) == 3:
                ts, user, msg = parts
                messages.append({
                    'ts': ts,
                    'user': user,
                    'msg': msg,
                })
except FileNotFoundError:
    messages = []
except Exception as e:
    respond(500, {"error": "failed_to_read"})
    raise SystemExit

# 直近limit件に絞る
messages = messages[-limit:]
respond(200, {"messages": messages})
