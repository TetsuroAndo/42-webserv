"""
GET / POST 混在 + keep-alive + 軽負荷並列テスト（同期版）

目的:
- keep-alive 接続を維持した状態でもレイテンシが劣化しない
- GET / POST を混ぜても相互干渉しない
- クライアント数・POST body サイズで極端な劣化が起きない
- 各ステータスコード（300, 400, 500など）の耐久テスト
"""

import math
import random
import time
from collections import defaultdict
from concurrent.futures import ThreadPoolExecutor, as_completed
from pathlib import Path

import pytest
import requests
import sys
import subprocess
import re


# =========================
# 設定値
# =========================
GET_RATIO = 0.8
REQUESTS_PER_CLIENT = 30
THINK_TIME_RANGE = (0.0, 0.002)  # 秒


# =========================
# 統計ユーティリティ
# =========================
def percentile(values, p):
    if not values:
        return None
    values = sorted(values)
    k = (len(values) - 1) * (p / 100)
    f = math.floor(k)
    c = math.ceil(k)
    if f == c:
        return values[int(k)]
    return values[f] * (c - k) + values[c] * (k - f)


# =========================
# ワーカークライアント
# =========================
def worker(
        base_url: str,
        post_body: bytes,
        results: dict,
):
    # 1 worker = 1 keep-alive 接続
    session = requests.Session()

    for _ in range(REQUESTS_PER_CLIENT):
        is_get = random.random() < GET_RATIO
        start = time.perf_counter()

        if is_get:
            r = session.get(f"{base_url}/")
            kind = "get"
        else:
            r = session.post(
                f"{base_url}/upload",
                data=post_body,
                headers={"Content-Type": "text/plain"},
            )
            kind = "post"

        elapsed_ms = (time.perf_counter() - start) * 1000
        results[kind].append(elapsed_ms)
        results["status"].append(r.status_code)

        time.sleep(random.uniform(*THINK_TIME_RANGE))


# =========================
# テスト本体
# =========================
@pytest.mark.config("valid/post.yaml")
@pytest.mark.parametrize("num_clients", [10, 50, 100, 500])
@pytest.mark.parametrize(
    "post_body_size",
    [
        128,        # 128 B
        4 * 1024,   # 4 KB
        32 * 1024,  # 32 KB
        64 * 1024,  # 64 KB
        128 * 1024, # 128 KB
        256 * 1024  # 256 KB
    ],
)
def test_mixed_get_post_keepalive_under_load(
        managed_server,
        num_clients,
        post_body_size,
):
    base_url = managed_server["base_url"]
    post_body = b"x" * post_body_size

    results = defaultdict(list)

    print(
        f"\n>>> 実行開始: POST body size = {post_body_size}B, クライアント数 = {num_clients}"
    )
    sys.stdout.flush()

    try:
        with ThreadPoolExecutor(max_workers=num_clients) as executor:
            futures = [
                executor.submit(worker, base_url, post_body, results)
                for _ in range(num_clients)
            ]
            for f in as_completed(futures):
                f.result()

        # =========================
        # 集計
        # =========================
        get_lat = results["get"]
        post_lat = results["post"]

        p95_get = percentile(get_lat, 95)
        p95_post = percentile(post_lat, 95)
        p99_get = percentile(get_lat, 99)
        p99_post = percentile(post_lat, 99)

        error_rate = sum(
            1 for s in results["status"] if s >= 500
        ) / len(results["status"])

        print(
            f"[clients={num_clients} body={post_body_size}] "
            f"GET p95={p95_get:.2f}ms p99={p99_get:.2f}ms | "
            f"POST p95={p95_post:.2f}ms p99={p99_post:.2f}ms | "
            f"errors={error_rate:.4%}"
        )

        # =========================
        # assert
        # =========================
        assert error_rate < 0.05, f"Error rate too high: {error_rate:.4%}"

        if num_clients <= 10:
            assert p95_get < 60, f"p95_get exceeded: {p95_get:.2f}ms"
            assert p95_post < 70, f"p95_post exceeded: {p95_post:.2f}ms"
            assert p99_get < 100, f"p99_get exceeded: {p99_get:.2f}ms"
            assert p99_post < 100, f"p99_post exceeded: {p99_post:.2f}ms"
        elif num_clients <= 50:
            assert p95_get < 240, f"p95_get exceeded: {p95_get:.2f}ms"
            assert p95_post < 270, f"p95_post exceeded: {p95_post:.2f}ms"
            assert p99_get < 580, f"p99_get exceeded: {p99_get:.2f}ms"
            assert p99_post < 600, f"p99_post exceeded: {p99_post:.2f}ms"
        elif num_clients <= 100:
            assert p95_get < 300, f"p95_get exceeded: {p95_get:.2f}ms"
            assert p95_post < 400, f"p95_post exceeded: {p95_post:.2f}ms"
            assert p99_get < 1500, f"p99_get exceeded: {p99_get:.2f}ms"
            assert p99_post < 1600, f"p99_post exceeded: {p99_post:.2f}ms"
        else:  # num_clients >= 500
            assert p95_get < 700, f"p95_get exceeded: {p95_get:.2f}ms"
            assert p95_post < 800, f"p95_post exceeded: {p95_post:.2f}ms"
            assert p99_get < 3000, f"p99_get exceeded: {p99_get:.2f}ms"
            assert p99_post < 3000, f"p99_post exceeded: {p99_post:.2f}ms"

    finally:
        upload_dir = Path("test/test_www/uploads")
        if upload_dir.exists():
            for entry in upload_dir.iterdir():
                if entry.name == ".gitignore":
                    continue
                if entry.is_file():
                    entry.unlink()


# =========================
# ステータスコード別耐久テスト用ワーカー
# =========================
def status_code_worker(
        base_url: str,
        target_status: int,
        request_func,
        results: dict,
        requests_per_client: int = 50,
):
    """特定のステータスコードを期待するリクエストを送信するワーカー"""
    session = requests.Session()

    for _ in range(requests_per_client):
        start = time.perf_counter()
        try:
            r = request_func(session, base_url)
            elapsed_ms = (time.perf_counter() - start) * 1000
            results["latency"].append(elapsed_ms)
            results["status"].append(r.status_code)
            results["expected_status"].append(target_status)
            results["success"].append(r.status_code == target_status)
        except Exception as e:
            elapsed_ms = (time.perf_counter() - start) * 1000
            results["latency"].append(elapsed_ms)
            results["status"].append(0)
            results["expected_status"].append(target_status)
            results["success"].append(False)
            results["errors"].append(str(e))

        time.sleep(random.uniform(0.0, 0.001))


# =========================
# 300系リダイレクトの耐久テスト
# =========================
@pytest.mark.config("valid/config_301.yaml")
@pytest.mark.parametrize("num_clients", [10, 50, 100])
def test_stress_redirect_301(managed_server, num_clients):
    """301リダイレクトの耐久テスト"""
    base_url = managed_server["base_url"]
    results = defaultdict(list)

    def request_func(session, url):
        return session.get(f"{url}/old-path", allow_redirects=False)

    print(f"\n>>> 301リダイレクト耐久テスト開始: クライアント数 = {num_clients}")
    sys.stdout.flush()

    with ThreadPoolExecutor(max_workers=num_clients) as executor:
        futures = [
            executor.submit(status_code_worker, base_url, 301, request_func, results)
            for _ in range(num_clients)
        ]
        for f in as_completed(futures):
            f.result()

    success_rate = sum(results["success"]) / len(results["success"]) if results["success"] else 0
    p95_latency = percentile(results["latency"], 95) if results["latency"] else None

    print(
        f"[301 redirect clients={num_clients}] "
        f"success_rate={success_rate:.4%} | "
        f"p95_latency={p95_latency:.2f}ms"
    )

    assert success_rate >= 0.99, f"301リダイレクト成功率が低すぎます: {success_rate:.4%}"
    assert p95_latency < 100, f"p95レイテンシが高すぎます: {p95_latency:.2f}ms"


@pytest.mark.config("valid/config_302.yaml")
@pytest.mark.parametrize("num_clients", [10, 50, 100])
def test_stress_redirect_302(managed_server, num_clients):
    """302リダイレクトの耐久テスト"""
    base_url = managed_server["base_url"]
    results = defaultdict(list)

    def request_func(session, url):
        return session.get(f"{url}/temp-old", allow_redirects=False)

    print(f"\n>>> 302リダイレクト耐久テスト開始: クライアント数 = {num_clients}")
    sys.stdout.flush()

    with ThreadPoolExecutor(max_workers=num_clients) as executor:
        futures = [
            executor.submit(status_code_worker, base_url, 302, request_func, results)
            for _ in range(num_clients)
        ]
        for f in as_completed(futures):
            f.result()

    success_rate = sum(results["success"]) / len(results["success"]) if results["success"] else 0
    p95_latency = percentile(results["latency"], 95) if results["latency"] else None

    print(
        f"[302 redirect clients={num_clients}] "
        f"success_rate={success_rate:.4%} | "
        f"p95_latency={p95_latency:.2f}ms"
    )

    assert success_rate >= 0.99, f"302リダイレクト成功率が低すぎます: {success_rate:.4%}"
    assert p95_latency < 100, f"p95レイテンシが高すぎます: {p95_latency:.2f}ms"


# =========================
# 400系エラーの耐久テスト
# =========================
@pytest.mark.config("valid/config_basic_get.yaml")
@pytest.mark.parametrize("num_clients", [10, 50, 100])
def test_stress_404_not_found(managed_server, num_clients):
    """404 Not Foundの耐久テスト"""
    base_url = managed_server["base_url"]
    results = defaultdict(list)

    def request_func(session, url):
        # 存在しないファイルへのリクエスト
        return session.get(f"{url}/non-existent-{random.randint(1000, 9999)}.txt")

    print(f"\n>>> 404 Not Found耐久テスト開始: クライアント数 = {num_clients}")
    sys.stdout.flush()

    with ThreadPoolExecutor(max_workers=num_clients) as executor:
        futures = [
            executor.submit(status_code_worker, base_url, 404, request_func, results)
            for _ in range(num_clients)
        ]
        for f in as_completed(futures):
            f.result()

    success_rate = sum(results["success"]) / len(results["success"]) if results["success"] else 0
    p95_latency = percentile(results["latency"], 95) if results["latency"] else None

    print(
        f"[404 Not Found clients={num_clients}] "
        f"success_rate={success_rate:.4%} | "
        f"p95_latency={p95_latency:.2f}ms"
    )

    assert success_rate >= 0.99, f"404エラー成功率が低すぎます: {success_rate:.4%}"
    assert p95_latency < 100, f"p95レイテンシが高すぎます: {p95_latency:.2f}ms"


@pytest.mark.config("valid/config_autoindex_off.yaml")
@pytest.mark.parametrize("num_clients", [10, 50, 100])
def test_stress_403_forbidden(managed_server, num_clients):
    """403 Forbiddenの耐久テスト"""
    base_url = managed_server["base_url"]
    results = defaultdict(list)

    def request_func(session, url):
        # autoindex offでディレクトリにアクセス
        return session.get(f"{url}/")

    print(f"\n>>> 403 Forbidden耐久テスト開始: クライアント数 = {num_clients}")
    sys.stdout.flush()

    with ThreadPoolExecutor(max_workers=num_clients) as executor:
        futures = [
            executor.submit(status_code_worker, base_url, 403, request_func, results)
            for _ in range(num_clients)
        ]
        for f in as_completed(futures):
            f.result()

    success_rate = sum(results["success"]) / len(results["success"]) if results["success"] else 0
    p95_latency = percentile(results["latency"], 95) if results["latency"] else None

    print(
        f"[403 Forbidden clients={num_clients}] "
        f"success_rate={success_rate:.4%} | "
        f"p95_latency={p95_latency:.2f}ms"
    )

    assert success_rate >= 0.99, f"403エラー成功率が低すぎます: {success_rate:.4%}"
    assert p95_latency < 100, f"p95レイテンシが高すぎます: {p95_latency:.2f}ms"


@pytest.mark.config("valid/config_basic_get.yaml")
@pytest.mark.parametrize("num_clients", [10, 50, 100])
def test_stress_405_method_not_allowed(managed_server, num_clients):
    """405 Method Not Allowedの耐久テスト"""
    base_url = managed_server["base_url"]
    results = defaultdict(list)

    def request_func(session, url):
        # GETが許可されていないパスにPOSTを送信
        return session.post(f"{url}/not_allow_get/hello.txt", data=b"test")

    print(f"\n>>> 405 Method Not Allowed耐久テスト開始: クライアント数 = {num_clients}")
    sys.stdout.flush()

    with ThreadPoolExecutor(max_workers=num_clients) as executor:
        futures = [
            executor.submit(status_code_worker, base_url, 405, request_func, results)
            for _ in range(num_clients)
        ]
        for f in as_completed(futures):
            f.result()

    success_rate = sum(results["success"]) / len(results["success"]) if results["success"] else 0
    p95_latency = percentile(results["latency"], 95) if results["latency"] else None

    print(
        f"[405 Method Not Allowed clients={num_clients}] "
        f"success_rate={success_rate:.4%} | "
        f"p95_latency={p95_latency:.2f}ms"
    )

    assert success_rate >= 0.99, f"405エラー成功率が低すぎます: {success_rate:.4%}"
    assert p95_latency < 100, f"p95レイテンシが高すぎます: {p95_latency:.2f}ms"


@pytest.mark.config("valid/post_test.yaml")
@pytest.mark.parametrize("num_clients", [10, 50, 100])
def test_stress_413_payload_too_large(managed_server, num_clients):
    """413 Payload Too Largeの耐久テスト"""
    base_url = managed_server["base_url"]
    results = defaultdict(list)

    def request_func(session, url):
        # 2KBを超えるペイロードを送信（maxRequestBodySize: 1KB）
        body = b"x" * 2048
        return session.post(
            f"{url}/upload",
            data=body,
            headers={"Content-Type": "application/octet-stream"}
        )

    print(f"\n>>> 413 Payload Too Large耐久テスト開始: クライアント数 = {num_clients}")
    sys.stdout.flush()

    with ThreadPoolExecutor(max_workers=num_clients) as executor:
        futures = [
            executor.submit(status_code_worker, base_url, 413, request_func, results)
            for _ in range(num_clients)
        ]
        for f in as_completed(futures):
            f.result()

    success_rate = sum(results["success"]) / len(results["success"]) if results["success"] else 0
    p95_latency = percentile(results["latency"], 95) if results["latency"] else None

    print(
        f"[413 Payload Too Large clients={num_clients}] "
        f"success_rate={success_rate:.4%} | "
        f"p95_latency={p95_latency:.2f}ms"
    )

    assert success_rate >= 0.99, f"413エラー成功率が低すぎます: {success_rate:.4%}"
    assert p95_latency < 200, f"p95レイテンシが高すぎます: {p95_latency:.2f}ms"


@pytest.mark.config("valid/post_test_content-length.yaml")
@pytest.mark.parametrize("num_clients", [10, 50])
def test_stress_400_bad_request(managed_server, num_clients):
    """400 Bad Requestの耐久テスト（Content-Length不一致）"""
    host = "127.0.0.1"
    port = managed_server["port"]
    results = defaultdict(list)
    requests_per_client = 20

    def worker_400():
        """400エラーを生成するワーカー"""
        for _ in range(requests_per_client):
            start = time.perf_counter()
            try:
                # Content-Length不一致のリクエストを送信
                length = random.randint(0, 20)
                http_request = (
                    f"POST /upload HTTP/1.1\\r\\n"
                    f"Host: {host}:{port}\\r\\n"
                    f"Content-Type: text/plain\\r\\n"
                    f"Content-Length: {length}\\r\\n"
                    f"Connection: close\\r\\n"
                    f"\\r\\n"
                    f"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                )

                cmd = f"printf '%b' '{http_request}' | curl --no-buffer --raw -s telnet://{host}:{port}"
                result = subprocess.run(
                    cmd,
                    shell=True,
                    capture_output=True,
                    text=True,
                    timeout=5
                )

                elapsed_ms = (time.perf_counter() - start) * 1000
                output = result.stderr + result.stdout
                status_match = re.search(r'HTTP/1\.\d+\s+(\d+)', output)

                if status_match:
                    status_code = int(status_match.group(1))
                    results["latency"].append(elapsed_ms)
                    results["status"].append(status_code)
                    results["expected_status"].append(400)
                    results["success"].append(status_code == 400)
                else:
                    results["latency"].append(elapsed_ms)
                    results["status"].append(0)
                    results["expected_status"].append(400)
                    results["success"].append(False)
                    results["errors"].append(f"Status code not found: {output}")
            except Exception as e:
                elapsed_ms = (time.perf_counter() - start) * 1000
                results["latency"].append(elapsed_ms)
                results["status"].append(0)
                results["expected_status"].append(400)
                results["success"].append(False)
                results["errors"].append(str(e))

            time.sleep(random.uniform(0.0, 0.001))

    print(f"\n>>> 400 Bad Request耐久テスト開始: クライアント数 = {num_clients}")
    sys.stdout.flush()

    with ThreadPoolExecutor(max_workers=num_clients) as executor:
        futures = [
            executor.submit(worker_400)
            for _ in range(num_clients)
        ]
        for f in as_completed(futures):
            f.result()

    success_rate = sum(results["success"]) / len(results["success"]) if results["success"] else 0
    p95_latency = percentile(results["latency"], 95) if results["latency"] else None

    print(
        f"[400 Bad Request clients={num_clients}] "
        f"success_rate={success_rate:.4%} | "
        f"p95_latency={p95_latency:.2f}ms"
    )

    assert success_rate >= 0.95, f"400エラー成功率が低すぎます: {success_rate:.4%}"


# =========================
# 500系エラーの耐久テスト
# =========================
@pytest.mark.config("valid/cgi.yaml")
@pytest.mark.parametrize("num_clients", [10, 50, 100])
def test_stress_500_internal_server_error(managed_server, num_clients):
    """500 Internal Server Errorの耐久テスト（CGIエラー）"""
    base_url = managed_server["base_url"]
    results = defaultdict(list)

    def request_func(session, url):
        # 失敗するCGIスクリプトを実行
        return session.get(f"{url}/cgi-bin/fail.sh")

    print(f"\n>>> 500 Internal Server Error耐久テスト開始: クライアント数 = {num_clients}")
    sys.stdout.flush()

    with ThreadPoolExecutor(max_workers=num_clients) as executor:
        futures = [
            executor.submit(status_code_worker, base_url, 500, request_func, results)
            for _ in range(num_clients)
        ]
        for f in as_completed(futures):
            f.result()

    success_rate = sum(results["success"]) / len(results["success"]) if results["success"] else 0
    p95_latency = percentile(results["latency"], 95) if results["latency"] else None

    print(
        f"[500 Internal Server Error clients={num_clients}] "
        f"success_rate={success_rate:.4%} | "
        f"p95_latency={p95_latency:.2f}ms"
    )

    assert success_rate >= 0.99, f"500エラー成功率が低すぎます: {success_rate:.4%}"
    assert p95_latency < 500, f"p95レイテンシが高すぎます: {p95_latency:.2f}ms"
