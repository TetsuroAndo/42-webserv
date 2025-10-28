"""
Keep-Alive接続処理のテスト
"""
import pytest
import socket
import time


class TestKeepAlive:
    @pytest.mark.config("valid/config_keepalive.yaml")
    def test_multiple_requests_on_same_connection(self, managed_server):
        """
        同じ接続で複数のリクエストが処理できることを確認
        """
        host, port = "127.0.0.1", 8080
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.settimeout(5)
        sock.connect((host, port))

        try:
            for i in range(3):
                request = b"GET / HTTP/1.1\r\n"
                request += b"Host: 127.0.0.1:8080\r\n"
                request += b"Connection: keep-alive\r\n"
                request += b"\r\n"

                sock.sendall(request)

                # レスポンスを受信
                response = sock.recv(4096)
                assert len(response) > 0, f"Response {i+1} should be received"

                # 接続が維持されていることを確認
                assert b"200 OK" in response or b"HTTP/1.1" in response

                print(f"Request {i+1} completed successfully")
        finally:
            sock.close()

    @pytest.mark.config("valid/config_keepalive.yaml")
    def test_connection_close_header(self, managed_server):
        """
        Connection: close ヘッダーで接続が閉じられることを確認
        """
        host, port = "127.0.0.1", 8080
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.settimeout(5)
        sock.connect((host, port))

        try:
            # 1回目のリクエスト - keep-alive
            request1 = b"GET / HTTP/1.1\r\n"
            request1 += b"Host: 127.0.0.1:8080\r\n"
            request1 += b"Connection: keep-alive\r\n"
            request1 += b"\r\n"

            sock.sendall(request1)
            response1 = sock.recv(4096)
            assert len(response1) > 0

            # 2回目のリクエスト - close
            request2 = b"GET / HTTP/1.1\r\n"
            request2 += b"Host: 127.0.0.1:8080\r\n"
            request2 += b"Connection: close\r\n"
            request2 += b"\r\n"

            sock.sendall(request2)
            response2 = sock.recv(4096)
            assert len(response2) > 0

            # 3回目のリクエストを試みる（接続は閉じられているはず）
            time.sleep(0.1)
            try:
                request3 = b"GET / HTTP/1.1\r\n"
                request3 += b"Host: 127.0.0.1:8080\r\n"
                request3 += b"\r\n"

                sock.sendall(request3)
                sock.settimeout(1)
                response3 = sock.recv(4096)

                # 接続が閉じられていた場合、エラーが発生する
                if len(response3) == 0:
                    # 接続が閉じられている（期待通り）
                    pass
                else:
                    pytest.fail("Connection should be closed after 'Connection: close'")
            except (BrokenPipeError, ConnectionResetError, OSError, socket.timeout):
                # 接続が閉じられている（期待通り）
                pass
        finally:
            sock.close()

    @pytest.mark.config("valid/config_keepalive.yaml")
    def test_context_reset_between_requests(self, managed_server):
        """
        リクエスト間にコンテキストが適切にリセットされることを確認
        """
        host, port = "127.0.0.1", 8080
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.settimeout(5)
        sock.connect((host, port))

        try:
            # 1回目のリクエスト - 存在するファイル
            request1 = b"GET /hello.txt HTTP/1.1\r\n"
            request1 += b"Host: 127.0.0.1:8080\r\n"
            request1 += b"Connection: keep-alive\r\n"
            request1 += b"\r\n"

            sock.sendall(request1)
            response1 = sock.recv(4096)
            assert b"200" in response1 or b"404" in response1
            print("Request 1 completed")

            time.sleep(0.2)  # 少し待機

            # 2回目のリクエスト - 別のファイル
            request2 = b"GET /index.html HTTP/1.1\r\n"
            request2 += b"Host: 127.0.0.1:8080\r\n"
            request2 += b"Connection: keep-alive\r\n"
            request2 += b"\r\n"

            sock.sendall(request2)
            response2 = sock.recv(4096)
            assert b"200" in response2 or b"404" in response2
            print("Request 2 completed")

            # 3回目のリクエスト
            request3 = b"GET / HTTP/1.1\r\n"
            request3 += b"Host: 127.0.0.1:8080\r\n"
            request3 += b"Connection: keep-alive\r\n"
            request3 += b"\r\n"

            sock.sendall(request3)
            response3 = sock.recv(4096)
            assert len(response3) > 0
            print("Request 3 completed")

        finally:
            sock.close()

    @pytest.mark.config("valid/config_keepalive.yaml")
    def test_error_response_with_keepalive(self, managed_server):
        """
        エラーレスポンス後もKeep-Aliveが継続されることを確認
        """
        host, port = "127.0.0.1", 8080
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.settimeout(5)
        sock.connect((host, port))

        try:
            # 1回目のリクエスト - 存在しないファイル（404エラー）
            request1 = b"GET /nonexistent_file.html HTTP/1.1\r\n"
            request1 += b"Host: 127.0.0.1:8080\r\n"
            request1 += b"Connection: keep-alive\r\n"
            request1 += b"\r\n"

            sock.sendall(request1)
            response1 = sock.recv(4096)
            assert b"404" in response1 or b"Not Found" in response1
            print("404 error received")

            time.sleep(0.2)

            # 2回目のリクエスト - 正常なリクエスト
            request2 = b"GET / HTTP/1.1\r\n"
            request2 += b"Host: 127.0.0.1:8080\r\n"
            request2 += b"Connection: keep-alive\r\n"
            request2 += b"\r\n"

            sock.sendall(request2)
            response2 = sock.recv(4096)
            assert len(response2) > 0
            assert b"HTTP/1.1" in response2

            print("Second request after 404 completed")

        finally:
            sock.close()

    @pytest.mark.config("valid/config_keepalive.yaml")
    def test_concurrent_keepalive_connections(self, managed_server):
        """
        複数のクライアントが同時にKeep-Alive接続を維持できることを確認
        """
        clients = []

        try:
            # 3つの同時接続を作成
            for i in range(3):
                sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                sock.settimeout(5)
                sock.connect(("127.0.0.1", 8080))
                clients.append(sock)

            # 各クライアントでリクエストを送信
            for idx, client in enumerate(clients):
                request = b"GET / HTTP/1.1\r\n"
                request += f"Host: 127.0.0.1:8080\r\n".encode()
                request += b"Connection: keep-alive\r\n"
                request += b"\r\n"

                client.sendall(request)
                response = client.recv(4096)
                assert len(response) > 0, f"Client {idx+1} should receive response"
                print(f"Client {idx+1} received response")

            # 各クライアントで2回目のリクエストを送信
            for idx, client in enumerate(clients):
                request = b"GET / HTTP/1.1\r\n"
                request += f"Host: 127.0.0.1:8080\r\n".encode()
                request += b"Connection: keep-alive\r\n"
                request += b"\r\n"

                client.sendall(request)
                response = client.recv(4096)
                assert len(response) > 0, f"Client {idx+1} should receive second response"
                print(f"Client {idx+1} received second response")

        finally:
            for client in clients:
                client.close()

    @pytest.mark.config("valid/config_keepalive.yaml")
    def test_keepalive_with_posts(self, managed_server):
        """
        POSTリクエストでKeep-Alive接続が正しく動作することを確認
        """
        host, port = "127.0.0.1", 8080
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.settimeout(5)
        sock.connect((host, port))

        try:
            # 1回目のリクエスト - GET
            request1 = b"GET / HTTP/1.1\r\n"
            request1 += b"Host: 127.0.0.1:8080\r\n"
            request1 += b"Connection: keep-alive\r\n"
            request1 += b"\r\n"

            sock.sendall(request1)
            response1 = sock.recv(4096)
            assert len(response1) > 0

            time.sleep(0.2)

            # 2回目のリクエスト - POST
            post_data = b"test=data&value=123"
            request2 = b"POST / HTTP/1.1\r\n"
            request2 += b"Host: 127.0.0.1:8080\r\n"
            request2 += b"Content-Type: application/x-www-form-urlencoded\r\n"
            request2 += f"Content-Length: {len(post_data)}\r\n".encode()
            request2 += b"Connection: keep-alive\r\n"
            request2 += b"\r\n"
            request2 += post_data

            sock.sendall(request2)
            response2 = sock.recv(4096)
            assert len(response2) > 0

            time.sleep(0.2)

            # 3回目のリクエスト - GET
            request3 = b"GET / HTTP/1.1\r\n"
            request3 += b"Host: 127.0.0.1:8080\r\n"
            request3 += b"Connection: keep-alive\r\n"
            request3 += b"\r\n"

            sock.sendall(request3)
            response3 = sock.recv(4096)
            assert len(response3) > 0

            print("POST and GET requests on same connection completed")

        finally:
            sock.close()
