#!/usr/bin/python3
# -*- coding: utf-8 -*-

import cgitb, html, datetime
import auth, storage

cgitb.enable()

# --- 認証チェック ---
username = auth.get_authenticated_user()
if not username:
    auth.redirect("login.py")
    exit()

# --- 認証成功 ---

auth.print_html_header("チャットルーム")
print(f"<h2>Webserv Chat (ようこそ, {username} さん)</h2>")
print("<a href='logout.py'>ログアウト</a>")

# チャットウィンドウとフォーム（初回は空、JSで描画）
print("<div class='chat-window' id='chat-window'></div>")
print("<div class='chat-form'>")
print("<form id='chat-form' method='POST' action='send_message.py' style='display: flex; width: 100%;'>")
print("<input type='text' id='message-input' name='message' style='flex-grow: 1; margin-right: 10px;' placeholder='メッセージを入力...' autocomplete='off'>")
print("<input type='submit' value='送信' class='btn'>")
print("</form>")
print("</div>")

# フロントJS
print("<script src='/js/chat.js'></script>")
auth.print_html_footer()
