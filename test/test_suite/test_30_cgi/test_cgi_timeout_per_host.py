"""
CGI timeout should follow each virtual host's timeoutSec.
"""

import os
import socket
import subprocess
import tempfile
import time
from contextlib import closing
from pathlib import Path
import psutil
import pytest
import requests

DEFAULT_HOST = "127.0.0.1"


def find_free_port():
    with closing(socket.socket(socket.AF_INET, socket.SOCK_STREAM)) as sock:
        sock.bind((DEFAULT_HOST, 0))
        sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        return sock.getsockname()[1]


def wait_for_port(host, port, timeout=5.0):
    deadline = time.time() + timeout
    while time.time() < deadline:
        try:
            with socket.create_connection((host, port), timeout=0.2):
                return True
        except (ConnectionRefusedError, socket.timeout, OSError):
            time.sleep(0.1)
    return False


@pytest.mark.integration
def test_cgi_timeout_per_host():
    port_zero = find_free_port()
    port_short = find_free_port()
    port_long = find_free_port()
    while port_long == port_short:
        port_long = find_free_port()

    project_root = Path(__file__).resolve().parents[3]
    webserv_bin = project_root / "webserv"
    cgi_root = project_root / "test/test_www/cgi-bin"

    config_text = f"""servers:
  - server:
      listens:
        - listen:
            interface: {DEFAULT_HOST}
            port: {port_short}
      maxRequestBodySize: 1048576
      timeoutSec: 1
      maxEvents: 128
      locations:
        - location:
            path: /cgi-bin
            root: {cgi_root}
            allowedMethods:
              - GET
              - POST
            interpreterPath:
              .py: /usr/bin/python3
  - server:
      listens:
        - listen:
            interface: {DEFAULT_HOST}
            port: {port_long}
      maxRequestBodySize: 1048576
      timeoutSec: 5
      maxEvents: 128
      locations:
        - location:
            path: /cgi-bin
            root: {cgi_root}
            allowedMethods:
              - GET
              - POST
            interpreterPath:
              .py: /usr/bin/python3
  - server:
      listens:
        - listen:
            interface: {DEFAULT_HOST}
            port: {port_zero}
      maxRequestBodySize: 1048576
      timeoutSec: 0
      maxEvents: 128
      locations:
        - location:
            path: /cgi-bin
            root: {cgi_root}
            allowedMethods:
              - GET
              - POST
            interpreterPath:
              .py: /usr/bin/python3
"""

    with tempfile.TemporaryDirectory(prefix="cgi_timeout_vhost_") as temp_dir:
        config_path = Path(temp_dir) / "cgi_timeout_vhost.yaml"
        config_path.write_text(config_text)

        proc = subprocess.Popen(
            [str(webserv_bin), str(config_path)],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
            preexec_fn=os.setsid,
        )

        try:
            ready_short = wait_for_port(DEFAULT_HOST, port_short)
            ready_long = wait_for_port(DEFAULT_HOST, port_long)
            if not (ready_short and ready_long):
                stdout, stderr = proc.communicate(timeout=1)
                pytest.fail(
                    "Server did not listen on both ports.\n"
                    f"stdout:\n{stdout}\n"
                    f"stderr:\n{stderr}\n"
                )

            zero_url = f"http://{DEFAULT_HOST}:{port_zero}/cgi-bin/wait_2sec.py"
            short_url = f"http://{DEFAULT_HOST}:{port_short}/cgi-bin/wait_2sec.py"
            long_url = f"http://{DEFAULT_HOST}:{port_long}/cgi-bin/wait_2sec.py"

            zero_resp = requests.get(zero_url, timeout=5)
            assert zero_resp.status_code == 408

            short_resp = requests.get(short_url, timeout=5)
            assert short_resp.status_code == 408

            long_resp = requests.get(long_url, timeout=5)
            assert long_resp.status_code == 200
            assert "Hello from CGI!" in long_resp.text
        finally:
            if proc.poll() is None:
                try:
                    parent = psutil.Process(proc.pid)
                    for child in parent.children(recursive=True):
                        child.kill()
                    parent.kill()
                except Exception:
                    proc.terminate()
                    try:
                        proc.wait(timeout=3)
                    except subprocess.TimeoutExpired:
                        proc.kill()
