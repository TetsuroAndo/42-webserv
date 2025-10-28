#!/usr/bin/python3
# -*- coding: utf-8 -*-

import cgi, cgitb, os, hashlib, html
import auth, storage, re

cgitb.enable()

def show_signup_form(error=""):
    auth.print_html_header("サインアップ")
    print("<h2>アカウント作成</h2>")
    if error:
        print(f"<p class='error'>{error}</p>")
    print("<div class='form-container'>")
    print("<form method='POST' action='signup.py'>")
    print("<div><label>ユーザー名:</label><input type='text' name='username'></div>")
    print("<div><label>パスワード:</label><input type='password' name='password'></div>")
    print("<div><input type='submit' value='作成' class='btn'></div>")
    print("</form>")
    print("<p><a href='login.py'>ログインはこちら</a></p>")
    print("</div>")
    auth.print_html_footer()

def handle_signup():
    form = cgi.FieldStorage()
    username = form.getvalue("username")
    password = form.getvalue("password")

    if not username or not password:
        show_signup_form("ユーザー名とパスワードを入力してください。")
        return
    # 入力バリデーション
    username = username.strip()
    if not re.fullmatch(r"[A-Za-z0-9_]{3,20}", username or ""):
        show_signup_form("ユーザー名は英数字とアンダースコアで3-20文字。")
        return
    if len(password) < 6 or len(password) > 128:
        show_signup_form("パスワードは6-128文字。")
        return

    # パスワードをハッシュ化 (簡易的)
    hashed_password = hashlib.sha256(password.encode()).hexdigest()

    # ユーザーが既に存在するかチェック
    try:
        with open(storage.USERS_DB, 'r') as f:
            for line in f:
                if line.startswith(f"{username}:"):
                    show_signup_form("そのユーザー名は既に使用されています。")
                    return
    except FileNotFoundError:
        pass # ファイルがなければ続行

    # 新規ユーザーを追記
    try:
        with open(storage.USERS_DB, 'a') as f:
            f.write(f"{username}:{hashed_password}\n")

        # ログインページにリダイレクト
        auth.redirect("login.py?msg=success")

    except Exception as e:
        show_signup_form(f"アカウント作成に失敗しました: {html.escape(str(e))}")

# --- メイン処理 ---
method = os.environ.get('REQUEST_METHOD')
if method == "POST":
    handle_signup()
else:
    show_signup_form()
