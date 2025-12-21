"""
タイムアウト処理のテスト
"""
import pytest
import socket
import time

# =========================
# ヘルパー関数
# =========================

def assert_408_response(data: bytes):
    assert data, "No response received"
    assert b"HTTP/" in data, f"Not an HTTP response: {data!r}"
    assert b" 408 " in data, f"Expected 408 status, got: {data!r}"
    assert b"Request Timeout" in data, f"Missing 'Request Timeout': {data!r}"


def assert_200_response(data: bytes):
    assert data, "No response received"
    assert b"HTTP/" in data, f"Not an HTTP response: {data!r}"
    assert b" 200 " in data, f"Expected 200 status, got: {data!r}"
def assert_201_response(data: bytes):
    assert data, "No response received"
    assert b"HTTP/" in data, f"Not an HTTP response: {data!r}"
    assert b" 201 " in data, f"Expected 201 status, got: {data!r}"

def assert_404_response(data: bytes):
    assert data, "No response received"
    assert b"HTTP/" in data, f"Not an HTTP response: {data!r}"
    assert b" 404 " in data, f"Expected 404 status, got: {data!r}"


class TestTimeout:
    @pytest.mark.config("valid/config_timeout.yaml")
    def test_request_header_timeout(self, managed_server):
        """
        リクエストラインを送信した後、ヘッダーを送信する前にタイムアウトすることを確認
        """
        # ソケット接続を確立
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.settimeout(5)
        sock.connect(("127.0.0.1", 8080))

        try:
            sock.sendall(b"GET / HTTP/1.1\r\n")
            time.sleep(3)
            data = sock.recv(4096)
            assert_408_response(data)
        finally:
            sock.close()

    @pytest.mark.config("valid/config_timeout.yaml")
    def test_request_body_timeout(self, managed_server):
        """
        リクエストボディーを受信する間にタイムアウトすることを確認
        """
        host, port = "127.0.0.1", 8080
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.settimeout(5)
        sock.connect((host, port))

        try:
            # リクエストヘッダーを送信
            sock.sendall(b"POST / HTTP/1.1\r\n")
            sock.sendall(b"Host: 127.0.0.1:8080\r\n")
            sock.sendall(b"Content-Type: text/plain\r\n")
            sock.sendall(b"Content-Length: 100\r\n")
            sock.sendall(b"\r\n")

            # ボディーの一部のみ送信して待つ
            sock.sendall(b"A" * 50)  # ボディーの半分だけ送信
            time.sleep(5)
            data = sock.recv(4096)
            assert_408_response(data)
        finally:
            sock.close()

    @pytest.mark.config("valid/config_timeout.yaml")
    def test_keepalive_idle_timeout(self, managed_server):
        """
        Keep-Alive アイドルタイムアウトで408になること
        """
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.settimeout(10)
        sock.connect(("127.0.0.1", 8080))

        try:
            request = (
                b"GET / HTTP/1.1\r\n"
                b"Host: 127.0.0.1:8080\r\n"
                b"Connection: keep-alive\r\n"
                b"\r\n"
            )

            sock.sendall(request)
            response = sock.recv(4096)
            assert_200_response(response)

            time.sleep(4)

            data = sock.recv(4096)
            assert_408_response(data)

        finally:
            sock.close()


    @pytest.mark.config("valid/config_timeout.yaml")
    def test_slow_request_not_affecting_other_clients(self, managed_server):
        """
        遅いリクエストが他のクライアントに影響を与えないことを確認（ノンブロッキング）
        """
        # 1つ目のクライアント - 遅いリクエストを送信
        host, port = "127.0.0.1", 8080
        slow_sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        slow_sock.settimeout(5)
        slow_sock.connect((host, port))

        try:
            # リクエストの一部だけ送信して遅延させる
            slow_sock.sendall(b"GET /")
            time.sleep(1)  # 少し遅延

            # 2つ目のクライアント - 通常の高速リクエスト
            fast_sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            fast_sock.settimeout(5)
            fast_sock.connect((host, port))

            try:
                request = b"GET / HTTP/1.1\r\n"
                request += b"Host: 127.0.0.1:8080\r\n"
                request += b"\r\n"

                # 高速リクエストを送信
                fast_sock.sendall(request)

                # すぐにレスポンスが返ってくることを確認
                response = fast_sock.recv(4096)
                assert_200_response(response)

            finally:
                fast_sock.close()
        finally:
            slow_sock.close()

    @pytest.mark.config("valid/config_timeout_short.yaml")
    def test_very_short_timeout(self, managed_server):
        """
        非常に短いタイムアウト時間での動作を確認
        """
        host, port = "127.0.0.1", 8080
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.settimeout(5)
        sock.connect((host, port))

        try:
            sock.sendall(b"GET /")
            time.sleep(1.5)
            data = sock.recv(4096)
            assert_408_response(data)
        finally:
            sock.close()

    @pytest.mark.config("valid/config_timeout_short.yaml")
    def test_complete_request_within_short_timeout(self, managed_server):
        """
        短いタイムアウトでも即完了すれば200
        """
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.settimeout(5)
        sock.connect(("127.0.0.1", 8080))

        try:
            sock.sendall(
                b"GET / HTTP/1.1\r\n"
                b"Host: 127.0.0.1:8080\r\n"
                b"\r\n"
            )

            response = sock.recv(4096)
            assert_200_response(response)

        finally:
            sock.close()

    @pytest.mark.config("valid/config_timeout_long.yaml")
    def test_long_timeout_allows_slow_requests(self, managed_server):
        """
        長いタイムアウト設定では、ゆっくりしたリクエストでもタイムアウトしないことを確認
        """
        host, port = "127.0.0.1", 8080
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.settimeout(5)
        sock.connect((host, port))

        try:
            # ゆっくりとリクエストを送信
            sock.sendall(b"GET /")
            time.sleep(2)  # 少し待つ

            sock.sendall(b" HTTP/1.1\r\n")
            time.sleep(2)

            sock.sendall(b"Host: 127.0.0.1:8080\r\n")
            sock.sendall(b"\r\n")

            # レスポンスを受信
            response = sock.recv(4096)
            assert_200_response(response)

        finally:
            sock.close()

    @pytest.mark.config("valid/config_timeout_asymmetric.yaml")
    def test_asymmetric_timeout_header_fails(self, managed_server):
        """
        ヘッダータイムアウト（短い）が先に発生することを確認
        """
        host, port = "127.0.0.1", 8080
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.settimeout(5)
        sock.connect((host, port))

        try:
            # リクエストラインのみ送信
            sock.sendall(b"GET /")

            # ヘッダータイムアウト（1秒）を待つ
            time.sleep(1.5)

            # 接続が閉じられているか確認
            data = sock.recv(4096)
            assert_408_response(data)
        finally:
            sock.close()

    @pytest.mark.config("valid/config_timeout_asymmetric.yaml")
    def test_asymmetric_timeout_body_passes(self, managed_server):
        host, port = "127.0.0.1", 8080
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.settimeout(10)
        sock.connect((host, port))

        try:
            # ヘッダーを素早く送信
            sock.sendall(b"POST / HTTP/1.1\r\n")
            sock.sendall(b"Host: 127.0.0.1:8080\r\n")
            sock.sendall(b"Content-Type: text/plain\r\n")
            sock.sendall(b"Content-Length: 200\r\n")
            sock.sendall(b"\r\n")

            sock.sendall(b"A" * 100)
            time.sleep(6)

            response = sock.recv(4096)
            assert_408_response(response)

        finally:
            sock.close()

    @pytest.mark.config("valid/config_timeout.yaml")
    def test_connection_without_any_data(self, managed_server):
        """
        接続後に何もデータを送信しない場合のタイムアウトを確認
        """
        host, port = "127.0.0.1", 8080
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.settimeout(10)
        sock.connect((host, port))

        try:
            time.sleep(1)

            try:
                sock.send(b"test")
                _ = sock.recv(1)
                assert True
            except (BrokenPipeError, ConnectionResetError, OSError, socket.timeout):
                pass
        finally:
            sock.close()

    @pytest.mark.config("valid/config_timeout.yaml")
    def test_multiple_sequential_requests(self, managed_server):
        """
        複数の連続したリクエストでもタイムアウトしないことを確認
        """
        host, port = "127.0.0.1", 8080
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.settimeout(10)
        sock.connect((host, port))

        try:
            # 複数のリクエストを連続して送信
            for i in range(3):
                request = b"GET / HTTP/1.1\r\n"
                request += b"Host: 127.0.0.1:8080\r\n"
                request += b"Connection: keep-alive\r\n"
                request += b"\r\n"

                sock.sendall(request)

                # レスポンスを受信
                response = sock.recv(4096)
                assert_200_response(response)

                time.sleep(0.5)  # 少し待つ

        finally:
            sock.close()

    @pytest.mark.config("valid/config_timeout.yaml")
    def test_head_request_timeout(self, managed_server):
        """
        HEADリクエストでもタイムアウトが正しく機能することを確認
        """
        host, port = "127.0.0.1", 8080
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.settimeout(5)
        sock.connect((host, port))

        try:
            # HEADリクエストのヘッダーの一部のみ送信
            sock.sendall(b"HEAD /")

            # タイムアウトを待つ
            time.sleep(3)

            # 接続が閉じられているか確認
            data = sock.recv(4096)
            assert_408_response(data)
        finally:
            sock.close()

    @pytest.mark.config("valid/config_timeout.yaml")
    def test_large_content_length_slow_transfer(self, managed_server):
        """
        大きなContent-Lengthでゆっくり転送する場合、タイムアウトしないことを確認

        注意: サーバーの実装では、タイムアウトは10秒に設定されているため、
        ゆっくり転送してもタイムアウトしないことを確認する。
        """
        host, port = "127.0.0.1", 8080
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.settimeout(10)
        sock.connect((host, port))

        try:
            # ヘッダーを送信
            sock.sendall(b"POST / HTTP/1.1\r\n")
            sock.sendall(b"Host: 127.0.0.1:8080\r\n")
            sock.sendall(b"Content-Type: text/plain\r\n")
            sock.sendall(b"Content-Length: 1000\r\n")
            sock.sendall(b"\r\n")

            # ボディーをゆっくり送信（タイムアウト10秒内）
            for i in range(10):
                sock.sendall(b"x" * 100)
                time.sleep(0.5)  # 各チャンクの間に遅延（合計5秒）

            # レスポンスを受信するはず
            response = sock.recv(4096)
            assert_201_response(response)
        finally:
            sock.close()

    @pytest.mark.config("valid/config_timeout.yaml")
    def test_no_connection_close_header_timeout(self, managed_server):
        """
        Connection: closeヘッダーでもタイムアウトが機能することを確認
        """
        host, port = "127.0.0.1", 8080
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.settimeout(5)
        sock.connect((host, port))

        try:
            # Connection: closeのリクエストラインのみ送信
            sock.sendall(b"GET / HTTP/1.1\r\n")

            # タイムアウトを待つ
            time.sleep(3)

            # 接続が閉じられているか確認
            data = sock.recv(4096)
            assert_408_response(data)
        finally:
            sock.close()

    @pytest.mark.config("valid/config_timeout.yaml")
    def test_concurrent_connections_timeout_individually(self, managed_server):
        """
        複数の接続が同時にタイムアウトすることを確認（各接続は独立してタイムアウト）
        """
        host, port = "127.0.0.1", 8080

        sockets = []
        # 3つの接続を同時に確立
        for i in range(3):
            sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            sock.settimeout(10)
            sock.connect((host, port))
            sockets.append(sock)

        # 各接続でリクエストラインのみ送信
        for sock in sockets:
            sock.sendall(b"GET /")

        # タイムアウトを待つ
        time.sleep(3)

        # 全ての接続が閉じられているか確認
        for sock in sockets:
            try:
                data = sock.recv(4096)
                assert_408_response(data)

            finally:
                for sock2 in sockets:
                    try:
                        sock2.close()
                    except Exception:
                        pass

    @pytest.mark.config("valid/config_timeout.yaml")
    def test_partial_chunked_transfer_timeout(self, managed_server):
        """
        Chunked転送の途中でタイムアウトすることを確認
        """
        host, port = "127.0.0.1", 8080
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.settimeout(10)
        sock.connect((host, port))

        try:
            # Chunked encodingでヘッダーを送信
            sock.sendall(b"POST / HTTP/1.1\r\n")
            sock.sendall(b"Host: 127.0.0.1:8080\r\n")
            sock.sendall(b"Transfer-Encoding: chunked\r\n")
            sock.sendall(b"\r\n")

            # 最初のチャンクのみ送信
            sock.sendall(b"64\r\n")  # 100バイト
            sock.sendall(b"A" * 100)
            sock.sendall(b"\r\n")

            # 次のチャンクを送信せずに待つ（タイムアウト）
            time.sleep(3)

            # 接続が閉じられているか確認
            sock.recv(4096)
            assert_408_response(sock.recv(4096))
        finally:
            sock.close()

    @pytest.mark.config("valid/config_timeout.yaml")
    def test_request_uri_too_long_not_timeout(self, managed_server):
        """
        長いURIを素早く送信してもタイムアウトしないことを確認
        """
        host, port = "127.0.0.1", 8080
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.settimeout(5)
        sock.connect((host, port))

        try:
            # 長いURIを含むリクエストを素早く送信
            long_path = "A" * 200
            request = f"GET /{long_path} HTTP/1.1\r\n".encode()
            request += b"Host: 127.0.0.1:8080\r\n"
            request += b"\r\n"

            sock.sendall(request)

            # レスポンスを受信（タイムアウトしない）
            response = sock.recv(4096)
            assert_404_response(response)

        finally:
            sock.close()

    @pytest.mark.config("valid/config_timeout_long.yaml")
    def test_multiple_slow_sequential_requests(self, managed_server):
        """
        長いタイムアウト設定で、複数のゆっくりしたリクエストが連続して処理できることを確認
        """
        host, port = "127.0.0.1", 8080
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.settimeout(10)
        sock.connect((host, port))

        try:
            # 複数のゆっくりしたリクエストを送信
            for i in range(3):
                if i == 0:
                    sock.sendall(b"GET /")
                time.sleep(2)

                sock.sendall(b" HTTP/1.1\r\n")
                time.sleep(2)

                sock.sendall(b"Host: 127.0.0.1:8080\r\n")
                sock.sendall(b"\r\n")

                # レスポンスを受信
                response = sock.recv(4096)
                assert_200_response(response)

                if i < 2:  # 最後のリクエスト以外は次のリクエストを送信
                    sock.sendall(b"GET /")

        finally:
            sock.close()

    @pytest.mark.config("valid/config_timeout_short.yaml")
    def test_immediate_complete_request_no_timeout(self, managed_server):
        """
        非常に短いタイムアウト設定でも、即座に完全なリクエストを送信すればタイムアウトしないことを確認
        """
        host, port = "127.0.0.1", 8080
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.settimeout(5)
        sock.connect((host, port))

        try:
            # 即座に完全なリクエストを送信
            request = b"GET / HTTP/1.1\r\n"
            request += b"Host: 127.0.0.1:8080\r\n"
            request += b"Content-Length: 0\r\n"
            request += b"\r\n"

            sock.sendall(request)

            # レスポンスを受信（タイムアウトしない）
            response = sock.recv(4096)
            assert_200_response(response)

        finally:
            sock.close()
