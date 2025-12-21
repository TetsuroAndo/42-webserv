"""
GET / POST 混在 + keep-alive + 軽負荷並列テスト（同期版）

目的:
- keep-alive 接続を維持した状態でもレイテンシが劣化しない
- GET / POST を混ぜても相互干渉しない
- クライアント数・POST body サイズで極端な劣化が起きない
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
        assert error_rate < 0.2, f"Error rate too high: {error_rate:.20%}"

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
            assert p95_get < 1300, f"p95_get exceeded: {p95_get:.2f}ms"
            assert p95_post < 1600, f"p95_post exceeded: {p95_post:.2f}ms"
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
