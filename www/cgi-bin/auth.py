#!/usr/bin/python3
# -*- coding: utf-8 -*-

import os, html
import storage

# セッションIDから認証済みユーザー名を取得する
def get_authenticated_user():
    session_id = os.environ.get('HTTP_X_WEBSERV_SESSION_ID')
    if not session_id:
        return None

    session_file_path = f"{storage.SESSIONS_DIR}/{session_id}.session"

    try:
        # セッションファイルからユーザー名を読み込む
        with open(session_file_path, 'r') as f:
            username = f.read().strip()
            if not username:
                return None
            return html.escape(username) # XSS対策
    except FileNotFoundError:
        return None
    except Exception:
        return None

# 汎用HTMLヘッダー
def print_html_header(title, refresh_sec=None):
    print("Content-Type: text/html; charset=utf-8\n\n")
    print("<!DOCTYPE html><html lang='ja'>")
    print("<head><meta charset='UTF-8'>")
    print(f"<title>{title} - Webserv Chat</title>")
    print("<link rel='stylesheet' href='/css/style.css'>")
    if refresh_sec:
        print(f"<meta http-equiv='refresh' content='{refresh_sec}'>")
    print("</head><body><div class='container'>")

# 汎用HTMLフッター
def print_html_footer():
    print("</div></body></html>")

# リダイレクト用
def redirect(location):
    print(f"Location: {location}\n\n")
