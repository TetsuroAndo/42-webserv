#!/usr/bin/python3
# -*- coding: utf-8 -*-

import os
import sys
import cgi
import json
import datetime
import fcntl
import mimetypes
import re

# add parent dir and cgi-bin dir to import storage/auth
sys.path.insert(0, os.path.dirname(os.path.dirname(__file__)))
sys.path.insert(0, os.path.join(os.path.dirname(os.path.dirname(__file__)), 'cgi-bin'))
import storage
import auth

# 予期せぬ例外でHTMLが返らないように（JSON固定）
# cgitbは無効化
# cgitb.enable()

print("Content-Type: application/json; charset=utf-8\n\n")

# 認証チェック（未ログイン時は匿名として送信）
username = auth.get_authenticated_user() or "anonymous"

method = os.environ.get('REQUEST_METHOD')
if method != 'POST':
    print(json.dumps({"ok": False, "error": "method_not_allowed"}, ensure_ascii=False))
    raise SystemExit

# 設定
MAX_SIZE = 5 * 1024 * 1024  # 5MB
ALLOWED_EXT = None  # すべて許可（必要なら{".png", ".jpg", ...}）

try:
    form = cgi.FieldStorage()
    caption = (form.getvalue("message") or "").strip()
    fileitem = form["file"] if "file" in form else None
except Exception:
    print(json.dumps({"ok": False, "error": "parse_failed"}, ensure_ascii=False))
    raise SystemExit

if fileitem is None or not getattr(fileitem, 'file', None):
    print(json.dumps({"ok": False, "error": "no_file"}, ensure_ascii=False))
    raise SystemExit

try:
    # ファイル名の正規化
    orig_filename = fileitem.filename or "upload.bin"
    orig_filename = os.path.basename(orig_filename)
    safe_name = re.sub(r"[^A-Za-z0-9._-]", "_", orig_filename)
    if not safe_name:
        safe_name = "upload.bin"

    ext = os.path.splitext(safe_name)[1].lower()
    if ALLOWED_EXT is not None and ext not in ALLOWED_EXT:
        print(json.dumps({"ok": False, "error": "extension_not_allowed"}, ensure_ascii=False))
        raise SystemExit

    # 保存先ディレクトリ
    uploads_dir = storage.UPLOADS_DIR
    if not uploads_dir.startswith('../') and not uploads_dir.startswith('./') and not uploads_dir.startswith('/'):
        # 念のため相対パス固定
        uploads_dir = os.path.join('..', uploads_dir)

    try:
        os.makedirs(uploads_dir, exist_ok=True)
    except TypeError:
        # Python3.6互換
        if not os.path.isdir(uploads_dir):
            os.makedirs(uploads_dir)

    # 一意なファイル名付与
    ts_for_name = datetime.datetime.now().strftime('%Y%m%d_%H%M%S_%f')
    final_name = f"{ts_for_name}_{safe_name}"
    abs_path = os.path.join(uploads_dir, final_name)

    # サイズ制限を見ながら保存
    written = 0
    try:
        with open(abs_path, 'wb') as out_f:
            while True:
                chunk = fileitem.file.read(64 * 1024)
                if not chunk:
                    break
                written += len(chunk)
                if written > MAX_SIZE:
                    out_f.close()
                    try:
                        os.remove(abs_path)
                    except Exception:
                        pass
                    print(json.dumps({"ok": False, "error": "file_too_large"}, ensure_ascii=False))
                    raise SystemExit
                out_f.write(chunk)
    except Exception:
        print(json.dumps({"ok": False, "error": "write_failed"}, ensure_ascii=False))
        raise SystemExit

    # URL と MIME 推定
    rel_url = f"/data/uploads/{final_name}"
    mime, _ = mimetypes.guess_type(final_name)
    mime = mime or "application/octet-stream"

    # メッセージログへ追記（JSONL）
    record = {
        "ts": datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S'),
        "user": username,
        "msg": caption[:500] if caption else "",
        "file": {
            "name": safe_name,
            "url": rel_url,
            "size": written,
            "mime": mime,
        },
    }

    try:
        line = json.dumps(record, ensure_ascii=False)
        with open(storage.MESSAGES_LOG, 'a') as f:
            fcntl.flock(f.fileno(), fcntl.LOCK_EX)
            f.write(line + "\n")
            fcntl.flock(f.fileno(), fcntl.LOCK_UN)
    except Exception:
        print(json.dumps({"ok": False, "error": "log_write_failed"}, ensure_ascii=False))
        raise SystemExit

    print(json.dumps({"ok": True, "record": record}, ensure_ascii=False))
except Exception as e:
    # 最後の砦: 予期せぬ例外もJSONで返す（簡易詳細付き）
    try:
        detail = str(e)
    except Exception:
        detail = ""
    print(json.dumps({"ok": False, "error": "unexpected_error", "detail": detail}, ensure_ascii=False))
