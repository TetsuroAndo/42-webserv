#!/usr/bin/python3
# -*- coding: utf-8 -*-

import cgi
import cgitb
import os
import datetime
import fcntl
import auth
import storage

cgitb.enable()

# --- 認証チェック ---
username = auth.get_authenticated_user()
if not username:
    auth.redirect("login.py")
    exit()

# --- POSTリクエストのみ処理 ---
method = os.environ.get('REQUEST_METHOD')
if method == "POST":
    form = cgi.FieldStorage()
    message = form.getvalue("message")

    if message:
        try:
            # タイムスタンプ
            ts = datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S')

            # 入力バリデーション
            message = message.strip()
            if not message:
                raise Exception('empty message')
            if len(message) > 500:
                message = message[:500]
            # 簡易サニタイズ（改行抑制）
            message = message.replace('\r', ' ').replace('\n', ' ')

            # メッセージをファイルに追記
            with open(storage.MESSAGES_LOG, 'a') as f:
                fcntl.flock(f.fileno(), fcntl.LOCK_EX)
                f.write(f"{ts}:{username}:{message}\n")
                fcntl.flock(f.fileno(), fcntl.LOCK_UN)

        except Exception:
            # エラーがあってもとりあえずチャット画面に戻す
            # 本来はエラーメッセージをchat.pyに渡すべき
            pass

# 処理が終わったらチャット画面にリダイレクト（後方互換）
auth.redirect("/chat.html")
