Load Testing for webserv

Overview
- This directory provides simple, dependency-free load tests for the server.
- Uses Python 3 asyncio and raw HTTP/1.1 to avoid external tools.

Files
- `load.py` — Async HTTP load generator (no third-party libs).
- `run.sh` — Helper to build, start the server, and run a few scenarios.

Quick Start
1) Build and start tests
   - `bash test/load_test/run.sh`

2) Run load generator manually
   - `python3 test/load_test/load.py --url http://127.0.0.1:8082/cgi-bin/simple.py --concurrency 50 --requests 5000`
   - POST example:
     `python3 test/load_test/load.py --url http://127.0.0.1:8082/cgi-bin/echo.py --method POST --data 'hello=world' --concurrency 50 --requests 2000`

Arguments (load.py)
- `--url`           Target URL (http only)
- `--method`        HTTP method (GET or POST; default GET)
- `--data`          POST body (form-encoded string)
- `--concurrency`   Number of concurrent connections (default 20)
- `--requests`      Total number of requests to send (default 1000)
- `--timeout`       Per-operation timeout seconds (default 5.0)

Output
- Prints totals, status distribution, RPS, and latency stats (avg, p50, p95, p99).

Notes
- Assumes the server sets `Content-Length` in responses (true for this project).
- Designed for localhost testing; adjust `--url`/config as needed.

