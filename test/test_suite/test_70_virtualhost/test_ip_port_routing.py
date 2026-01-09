"""
IP/Port-based VirtualHost routing test.
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


@pytest.fixture
def webserv_bin():
    test_dir = Path(__file__).parent.parent.parent
    project_root = test_dir.parent
    return str(project_root / "webserv")


@pytest.mark.integration
def test_ip_port_virtualhost_routing(webserv_bin):
    port_a = find_free_port()
    port_b = find_free_port()
    while port_b == port_a:
        port_b = find_free_port()

    host_a_contents = f"""
    Host A is running on IP: {DEFAULT_HOST} PORT: {port_a}
    """
    host_b_contents = f"""
    Host B is running on IP: {DEFAULT_HOST} PORT: {port_b}
    """

    with tempfile.TemporaryDirectory(prefix="vhost_test_") as temp_dir:
        root_a = os.path.join(temp_dir, "root_a")
        root_b = os.path.join(temp_dir, "root_b")
        os.makedirs(root_a)
        os.makedirs(root_b)

        index_a = os.path.join(root_a, "index.html")
        index_b = os.path.join(root_b, "index.html")
        with open(index_a, "w") as f:
            f.write(host_a_contents)
        with open(index_b, "w") as f:
            f.write(host_b_contents)
        print(f"index_a: {index_a}")
        print(f"index_b: {index_b}")
        with open(index_a, "r") as f:
            print(f"index_a contents: {f.read()}")
        with open(index_b, "r") as f:
            print(f"index_b contents: {f.read()}")

        config_path = os.path.join(temp_dir, "vhost.yaml")
        config_text = f"""servers:
  - server:
      listens:
        - listen:
            interface: {DEFAULT_HOST}
            port: {port_a}
      locations:
        - location:
            path: /
            root: {root_a}
            index: index.html
            autoindex: false
            allowedMethods:
              - GET
  - server:
      listens:
        - listen:
            interface: {DEFAULT_HOST}
            port: {port_b}
      locations:
        - location:
            path: /
            root: {root_b}
            index: index.html
            autoindex: false
            allowedMethods:
              - GET
"""
        with open(config_path, "w") as f:
            f.write(config_text)

        with open(config_path, "r") as f:
            print(f"config: \n\n{f.read()}")
        print(f"config_path: {config_path}")
        print(f"port_a: {port_a}, port_b: {port_b}")

        proc = subprocess.Popen(
            [webserv_bin, config_path],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
            preexec_fn=os.setsid,
        )

        try:
            # 空いているポートを取得
            ready_a = wait_for_port(DEFAULT_HOST, port_a)
            ready_b = wait_for_port(DEFAULT_HOST, port_b)
            if not (ready_a and ready_b):
                stdout, stderr = proc.communicate(timeout=1)
                pytest.fail(
                    "Server did not listen on both ports.\n"
                    f"stdout:\n{stdout}\n"
                    f"stderr:\n{stderr}\n"
                )

            # ポートにアクセスして内容を取得
            resp_a = requests.get(f"http://{DEFAULT_HOST}:{port_a}/", timeout=2)
            resp_b = requests.get(f"http://{DEFAULT_HOST}:{port_b}/", timeout=2)

            # ステータスコードが200であることを確認
            assert resp_a.status_code == 200
            assert resp_b.status_code == 200
            assert host_a_contents in resp_a.text
            print(f"resp_a contents: {resp_a.text}")
            assert host_b_contents in resp_b.text
            print(f"resp_b contents: {resp_b.text}")
            assert resp_a.text != resp_b.text
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
