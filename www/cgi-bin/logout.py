#!/usr/bin/python3
# -*- coding: utf-8 -*-

import os, cgitb
import auth, storage

cgitb.enable()

session_id = os.environ.get('HTTP_X_WEBSERV_SESSION_ID')
if session_id:
    session_file_path = f"{storage.SESSIONS_DIR}/{session_id}.session"
    try:
        os.remove(session_file_path) # セッションファイルを削除
    except FileNotFoundError:
        pass # 既に存在しなくてもOK
    except Exception:
        pass # エラーは握りつぶしてログアウト処理を続行

# ホームにリダイレクト
auth.redirect("/index.html")
