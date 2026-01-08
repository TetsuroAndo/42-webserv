"""
GET + Cookie / Session のテスト（Set-Cookie 基準）
"""

import pytest
import requests
import re
import time


def extract_session_id(set_cookie_header):
    if not set_cookie_header:
        return None
    m = re.search(r'sessionId=("[^"]+"|[^;]+)', set_cookie_header)
    if not m:
        return None
    return m.group(1)


class TestGETCookieSession:
    @pytest.mark.config("valid/config_session.yaml")
    def test_set_cookie_header_exists(self, managed_server):
        """
        初回アクセスで Set-Cookie が返り、
        sessionId が含まれていること
        """
        url = f"{managed_server['base_url']}/"

        r = requests.get(url)

        assert r.status_code == 200
        assert "Set-Cookie" in r.headers

        session_id = extract_session_id(r.headers.get("Set-Cookie"))
        assert session_id is not None

    @pytest.mark.config("valid/config_session.yaml")
    def test_session_cookie_is_reused(self, managed_server):
        """
        同じセッションが再利用される場合、
        Set-Cookie が返らない or 同じ sessionId が返る
        """
        base_url = managed_server["base_url"]
        session = requests.Session()

        # 1回目
        r1 = session.get(f"{base_url}/")
        assert r1.status_code == 200

        sid1 = extract_session_id(r1.headers.get("Set-Cookie"))
        assert sid1 is not None

        # 2回目（同じ Cookie を送る）
        r2 = session.get(f"{base_url}/")
        assert r2.status_code == 200

        sid2 = extract_session_id(r2.headers.get("Set-Cookie"))

        # 判断基準：
        # - Set-Cookie が返らない → 同一セッション
        # - 返ってきても sessionId が同じ → 同一セッション
        if sid2 is not None:
            assert sid1 == sid2

    @pytest.mark.config("valid/config_zero_sessionTimeoutSec.yaml")
    def test_session_timeout_zero_creates_new_session(self, managed_server):
        """
        sessionTimeout=0 の場合、
        Set-Cookie が返り、かつ sessionId が変わることで
        新しいセッションが作られたと判断する
        """
        base_url = managed_server["base_url"]
        session = requests.Session()

        time.sleep(0.1)

        # 1回目
        r1 = session.get(f"{base_url}/")
        assert r1.status_code == 200

        sid1 = extract_session_id(r1.headers.get("Set-Cookie"))
        assert sid1 is not None

        time.sleep(0.1)

        # 2回目
        r2 = session.get(f"{base_url}/")
        assert r2.status_code == 200

        sid2 = extract_session_id(r2.headers.get("Set-Cookie"))

        # 新しいセッションの判断基準：
        # - Set-Cookie が返ってくる
        # - sessionId が前回と違う
        assert sid2 is not None
        assert sid1 != sid2

    @pytest.mark.config("valid/config_vhost_session_timeout.yaml")
    def test_vhost_session_timeout_isolated(self, managed_server):
        """
        vhostごとに sessionTimeoutSec が異なる場合、
        それぞれのTimeoutが独立して適用される
        """
        base_url_1 = managed_server["base_url"]
        base_url_2 = f"http://127.0.0.2:{managed_server['port']}"
        base_url_3 = f"http://127.0.0.3:{managed_server['port']}"

        session_1 = requests.Session()
        session_2 = requests.Session()
        session_3 = requests.Session()

        r1 = session_1.get(f"{base_url_1}/")
        assert r1.status_code == 200
        sid1 = extract_session_id(r1.headers.get("Set-Cookie"))
        assert sid1 is not None
        print(f"sid1: {sid1}")

        r2 = session_2.get(f"{base_url_2}/")
        assert r2.status_code == 200
        sid2 = extract_session_id(r2.headers.get("Set-Cookie"))
        assert sid2 is not None
        print(f"sid2: {sid2}")

        r3 = session_3.get(f"{base_url_3}/")
        assert r3.status_code == 200
        sid3 = extract_session_id(r3.headers.get("Set-Cookie"))
        print(f"sid3: {sid3}")
        assert sid3 is not None

        time.sleep(0.1)

        r1b = session_1.get(f"{base_url_1}/")
        assert r1b.status_code == 200
        sid1b = extract_session_id(r1b.headers.get("Set-Cookie"))
        if sid1b is None:
            # Set-Cookieが返ってこない→セッション維持
            print("sid1b: セッション維持 Set-Cookieなし")
            assert True
        else:
            print(f"セッション維持: sid1b: {sid1b} == sid1: {sid1}")
            assert sid1b == sid1

        r3b = session_3.get(f"{base_url_3}/")
        assert r3b.status_code == 200
        sid3b = extract_session_id(r3b.headers.get("Set-Cookie"))
        print(f"sid3b: {sid3b}")
        assert sid3b is not None

        time.sleep(2)

        r1c = session_1.get(f"{base_url_1}/")
        assert r1c.status_code == 200
        sid1c = extract_session_id(r1c.headers.get("Set-Cookie"))
        print(f"sid1c: {sid1c}")
        assert sid1c is not None
        assert sid1c != sid1b

        r2b = session_2.get(f"{base_url_2}/")
        assert r2b.status_code == 200
        sid2b = extract_session_id(r2b.headers.get("Set-Cookie"))
        if sid2b is None:
            # Set-Cookieが返ってこない→セッション維持
            print("sid2b: セッション維持 Set-Cookieなし")
            assert True
        else:
            print(f"sid2b: セッション維持: sid2b: {sid2b} == sid2: {sid2}")
            assert sid2b == sid2
