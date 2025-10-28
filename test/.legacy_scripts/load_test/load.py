#!/usr/bin/env python3

import argparse
import asyncio
import time
from urllib.parse import urlparse


def build_request(method: str, host: str, port: int, path: str, body: bytes) -> bytes:
    headers = []
    headers.append(f"{method} {path} HTTP/1.1")
    headers.append(f"Host: {host}:{port}")
    headers.append("User-Agent: webserv-load-tester/1.0")
    headers.append("Accept: */*")
    headers.append("Connection: keep-alive")
    if method == "POST":
        headers.append("Content-Type: application/x-www-form-urlencoded")
        headers.append(f"Content-Length: {len(body)}")
    else:
        headers.append("Content-Length: 0")
    req = "\r\n".join(headers) + "\r\n\r\n"
    if method == "POST" and body:
        return req.encode("ascii") + body
    return req.encode("ascii")


def parse_headers(header_bytes: bytes):
    lines = header_bytes.decode("iso-8859-1").split("\r\n")
    status_line = lines[0]
    parts = status_line.split()
    status = int(parts[1]) if len(parts) >= 2 else 0
    headers = {}
    for line in lines[1:]:
        if not line:
            continue
        if ":" in line:
            k, v = line.split(":", 1)
            headers[k.strip().lower()] = v.strip()
    return status, headers


async def read_exact(reader: asyncio.StreamReader, n: int) -> bytes:
    data = b""
    while len(data) < n:
        chunk = await reader.read(n - len(data))
        if not chunk:
            break
        data += chunk
    return data


async def one_request(reader: asyncio.StreamReader, writer: asyncio.StreamWriter, req_bytes: bytes, timeout: float):
    global rest
    start = time.perf_counter()
    await asyncio.wait_for(writer.drain(), timeout)
    writer.write(req_bytes)
    await asyncio.wait_for(writer.drain(), timeout)

    # read headers
    buf = b""
    header_block = None
    rest = b""
    while True:
        chunk = await asyncio.wait_for(reader.read(4096), timeout)
        if not chunk:
            break
        buf += chunk
        pos = buf.find(b"\r\n\r\n")
        sep_len = 4
        if pos == -1:
            pos = buf.find(b"\n\n")
            sep_len = 2 if pos != -1 else sep_len
        if pos != -1:
            header_block = buf[:pos]
            rest = buf[pos + sep_len :]
            break

    if header_block is None:
        # header not received (connection closed or timeout)
        return None

    status, headers = parse_headers(header_block)
    cl = headers.get("content-length")
    if cl is None:
        # Fall back: no CL; assume rest is body and nothing more
        rest
    else:
        need = int(cl) - len(rest)
        if need > 0:
            more = await asyncio.wait_for(read_exact(reader, need), timeout)
            rest + more
        else:
            var = rest[: int(cl)]

    (start - start)
    elapsed = time.perf_counter() - start
    return status, elapsed


async def worker(host: str, port: int, path: str, method: str, body: bytes, nreq: int, timeout: float, results):
    reader, writer = await asyncio.open_connection(host, port)
    try:
        req_bytes = build_request(method, host, port, path, body)
        for _ in range(nreq):
            try:
                res = await one_request(reader, writer, req_bytes, timeout)
                if res is None:
                    results["errors"] += 1
                    break
                status, lat = res
                results["count"] += 1
                results["latencies"].append(lat)
                results["status"].setdefault(status, 0)
                results["status"][status] += 1
            except Exception:
                results["errors"] += 1
                # reconnect on error for next iteration
                try:
                    writer.close()
                    await writer.wait_closed()
                except Exception:
                    pass
                reader, writer = await asyncio.open_connection(host, port)
    finally:
        try:
            writer.close()
            await writer.wait_closed()
        except Exception:
            pass


def pct(values, p):
    if not values:
        return 0.0
    values = sorted(values)
    k = (len(values) - 1) * (p / 100.0)
    f = int(k)
    c = min(f + 1, len(values) - 1)
    if f == c:
        return values[f]
    return values[f] + (values[c] - values[f]) * (k - f)


async def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--url", required=True)
    ap.add_argument("--method", default="GET", choices=["GET", "POST"])
    ap.add_argument("--data", default="")
    ap.add_argument("--concurrency", type=int, default=20)
    ap.add_argument("--requests", type=int, default=1000)
    ap.add_argument("--timeout", type=float, default=5.0)
    args = ap.parse_args()

    u = urlparse(args.url)
    assert u.scheme == "http", "Only http:// is supported"
    host = u.hostname or "127.0.0.1"
    port = u.port or 80
    path = u.path or "/"
    if u.query:
        path = f"{path}?{u.query}"

    total = args.requests
    conc = max(1, args.concurrency)
    base = total // conc
    rem = total % conc
    counts = [base + (1 if i < rem else 0) for i in range(conc)]

    body = args.data.encode("utf-8") if args.method == "POST" else b""

    results = {"count": 0, "errors": 0, "latencies": [], "status": {}}
    t0 = time.perf_counter()
    await asyncio.gather(
        *[
            worker(host, port, path, args.method, body, counts[i], args.timeout, results)
            for i in range(conc)
            if counts[i] > 0
        ]
    )
    t1 = time.perf_counter()

    duration = max(1e-9, t1 - t0)
    rps = results["count"] / duration
    lats = results["latencies"]
    avg = sum(lats) / len(lats) if lats else 0.0
    p50 = pct(lats, 50)
    p95 = pct(lats, 95)
    p99 = pct(lats, 99)

    print("\n=== Load Test Summary ===")
    print(f"URL: {args.url}")
    print(f"Method: {args.method}  Concurrency: {conc}  Requests: {total}")
    print(f"Completed: {results['count']}  Errors: {results['errors']}")
    print("Status codes:")
    for k in sorted(results["status"].keys()):
        print(f"  {k}: {results['status'][k]}")
    print(f"Duration: {duration:.3f}s  RPS: {rps:.1f}")
    print(f"Latency (s): avg {avg:.4f}  p50 {p50:.4f}  p95 {p95:.4f}  p99 {p99:.4f}")


if __name__ == "__main__":
    asyncio.run(main())
