#!/bin/bash

# サーバーを起動
cd /Users/atomboy/42Toybox/0-Cursus/52-webserv
./webserv test/confs/valid/config_timeout.yaml &
SERVER_PID=$!

sleep 1

# クライアント接続を作成し、リクエストラインを送信
exec 3<>/dev/tcp/127.0.0.1/8080
echo -e "GET / HTTP/1.1\r\n" >&3

# サーバー側のプロセスを確認
echo "=== Server processes ==="
ps aux | grep webserv | grep -v grep

# 接続を確認
sleep 3

echo "=== After 3 seconds ==="
# lsofで接続を確認
lsof -i:8080 2>/dev/null | grep 127.0.0.1

# 接続にデータを送信してみる
echo -e "test" >&3

# lsofで接続を確認
lsof -i:8080 2>/dev/null | grep 127.0.0.1

exec 3<&-
exec 3>&-

# サーバーを終了
kill $SERVER_PID 2>/dev/null
wait $SERVER_PID 2>/dev/null

