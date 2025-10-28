#!/usr/bin/python3
# -*- coding: utf-8 -*-

import cgi, cgitb, os, hashlib, html
import auth, storage, re

cgitb.enable()

def show_login_form(error="", msg=""):
    auth.print_html_header("ログイン")
    print("<h2>ログイン</h2>")
    if error:
        print(f"<p class='error'>{error}</p>")
    if msg == "success":
        print("<p style='color:green;'>アカウントを作成しました。ログインしてください。</p>")

    print("<div class='form-container'>")
    print("<form method='POST' action='login.py'>")
    print("<div><label>ユーザー名:</label><input type='text' name='username'></div>")
    print("<div><label>パスワード:</label><input type='password' name='password'></div>")
    print("<div><input type='submit' value='ログイン' class='btn'></div>")
    print("</form>")
    print("<p><a href='signup.py'>アカウント作成はこちら</a></p>")
    print("</div>")
    auth.print_html_footer()

def handle_login():
    form = cgi.FieldStorage()
    username = form.getvalue("username")
    password = form.getvalue("password")

    if not username or not password:
        show_login_form("ユーザー名とパスワードを入力してください。")
        return
    # 入力バリデーション
    username = username.strip()
    if not re.fullmatch(r"[A-Za-z0-9_]{3,20}", username or ""):
        show_login_form("ユーザー名は英数字とアンダースコアで3-20文字。")
        return
    if len(password) < 6 or len(password) > 128:
        show_login_form("パスワードは6-128文字。")
        return

    hashed_password = hashlib.sha256(password.encode()).hexdigest()

    # ユーザー認証
    authenticated = False
    try:
        with open(storage.USERS_DB, 'r') as f:
            for line in f:
                parts = line.strip().split(':', 1)
                if len(parts) == 2 and parts[0] == username and parts[1] == hashed_password:
                    authenticated = True
                    break
    except FileNotFoundError:
        show_login_form("ログインに失敗しました。")
        return

    if not authenticated:
        show_login_form("ユーザー名またはパスワードが間違っています。")
        return

    # --- 認証成功 ---
    # Webservが発行したセッションIDを取得
    session_id = os.environ.get('HTTP_X_WEBSERV_SESSION_ID')
    if not session_id:
        show_login_form("セッションの取得に失敗しました。")
        return

    # セッションファイルを作成してユーザー名を保存
    try:
        session_file_path = f"{storage.SESSIONS_DIR}/{session_id}.session"
        with open(session_file_path, 'w') as f:
            f.write(username)

        # チャットページにリダイレクト
        auth.redirect("chat.py")

    except Exception as e:
        show_login_form(f"セッションの保存に失敗しました: {html.escape(str(e))}")


# --- メイン処理 ---
method = os.environ.get('REQUEST_METHOD')
form = cgi.FieldStorage()
msg = form.getvalue("msg", "") # signup.pyからのメッセージ

if method == "POST":
    handle_login()
else:
    show_login_form(msg=msg)
