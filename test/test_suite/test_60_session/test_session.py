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
    #@pytest.mark.skip(reason="このテストは30分かかるようになってるので、高速に実行する際はコンパイルをし直してください。")
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
