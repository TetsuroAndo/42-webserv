#!/usr/bin/env python3

import socket
import time

# --- 設定 ---
# テスト対象のサーバーのホストとポート
HOST = "localhost"
PORT = 8080  # config/default.yaml のポートに合わせてください

# タイムアウト（秒）
TIMEOUT = 2
# ----------------

def create_request(path, connection_header):
    """指定されたパスとConnectionヘッダーでHTTP/1.0リクエストを作成する"""
    request_lines = [
        "GET {} HTTP/1.0".format(path),
        "Host: {}:{}".format(HOST, PORT),
        "Connection: {}".format(connection_header),
        "User-Agent: KeepAlive-Tester/1.0",
        "\r\n"  # ヘッダーの終わり
    ]
    return "\r\n".join(request_lines)

def parse_response(response_bytes):
    """レスポンスをヘッダーとボディに分割し、ヘッダーを辞書として返す"""
    try:
        header_str, body = response_bytes.decode('utf-8').split('\r\n\r\n', 1)
    except ValueError:
        print("\033[91m[ERROR]\033[0m レスポンスの形式が不正です。ヘッダーとボディを分離できませんでした。")
        return None, None, None

    header_lines = header_str.split('\r\n')
    status_line = header_lines[0]
    headers = {}
    for line in header_lines[1:]:
        key, value = line.split(':', 1)
        headers[key.strip().lower()] = value.strip()

    return status_line, headers, body

def run_test():
    """Keep-Aliveのテストを実行する"""
    print("--- Keep-Alive Test Start ---")
    print("Target: http://{}:{}".format(HOST, PORT))

    # 1. サーバーに接続
    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.settimeout(TIMEOUT)
        sock.connect((HOST, PORT))
        print("\033[92m[SUCCESS]\033[0m サーバーへの接続に成功しました。")
        peer = sock.getpeername()
        print("  -> Connected to {}:{}".format(peer[0], peer[1]))
    except socket.error as e:
        print("\033[91m[FAIL]\033[0m サーバーへの接続に失敗しました: {}".format(e))
        return

    try:
        # 2. 1回目のリクエスト (Keep-Aliveを要求)
        print("\n[STEP 1] 1回目のリクエストを送信 (Connection: keep-alive)")
        request1 = create_request("/", "keep-alive")
        sock.sendall(request1.encode('utf-8'))
        print("  -> Sent request:\n---\n{}\n---".format(request1.strip()))

        # 1回目のレスポンスを受信
        response1_bytes = sock.recv(4096)
        if not response1_bytes:
            print("\033[91m[FAIL]\033[0m 1回目のレスポンスが空です。接続が閉じられた可能性があります。")
            return
        
        status, headers, body = parse_response(response1_bytes)
        print("  -> Received response:")
        print("     Status: {}".format(status))
        print("     Connection Header: {}".format(headers.get('connection', '(not found)')))

        if headers.get('connection', '').lower() != 'keep-alive':
            # HTTP/1.1のサーバーは Connection ヘッダーを返さない場合もあるので警告に留める
            print("\033[93m[WARN]\033[0m レスポンスに 'Connection: keep-alive' が含まれていません。")
            print("       HTTP/1.1仕様ではデフォルトでKeep-Aliveのため、テストを続行します。")

        # 3. 2回目のリクエスト (同じ接続を使用)
        print("\n[STEP 2] 2回目のリクエストを同じ接続で送信")
        # ウェブサーバーの実装によっては、即座に送信するとリクエストの区切りを
        # 正しく認識できない場合があるため、わずかな待機時間を設ける
        time.sleep(0.1) 
        
        request2 = create_request("/index.html", "close") # 最後に接続を閉じる
        sock.sendall(request2.encode('utf-8'))
        print("  -> Sent request:\n---\n{}\n---".format(request2.strip()))

        # 2回目のレスポンスを受信
        response2_bytes = sock.recv(4096)
        if not response2_bytes:
            print("\033[91m[FAIL]\033[0m 2回目のレスポンスが空です。Keep-Aliveが機能していない可能性があります。")
            return
            
        status2, _, _ = parse_response(response2_bytes)
        print("  -> Received response:")
        print("     Status: {}".format(status2))
        print("\033[92m[SUCCESS]\033[0m 2回目のレスポンスを正常に受信しました。")

        # 4. サーバーが接続を閉じたか確認
        print("\n[STEP 3] サーバーが接続を閉じるか確認 (Connection: close)")
        time.sleep(0.5) # サーバーがcloseするのを待つ
        final_data = sock.recv(1)
        if not final_data:
            print("\033[92m[SUCCESS]\033[0m サーバーが正常に接続を閉じました。")
        else:
            print("\033[93m[WARN]\033[0m サーバーが接続を閉じていません。")

    except socket.timeout:
        print("\033[91m[FAIL]\033[0m ソケットがタイムアウトしました。")
    except socket.error as e:
        print("\033[91m[FAIL]\033[0m ソケットエラーが発生しました: {}".format(e))
    except Exception as e:
        print("\033[91m[FAIL]\033[0m 不明なエラーが発生しました: {}".format(e))
    finally:
        sock.close()
        print("\n--- Test Finished ---")

if __name__ == "__main__":
    run_test()