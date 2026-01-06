"""
サーバーの正常起動テスト
"""
import socket
import subprocess
import tempfile
import time
from contextlib import closing
from pathlib import Path

import pytest
import requests
import yaml


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


def write_multi_listen_config(src_path, temp_dir):
    with open(src_path, "r") as f:
        config = yaml.safe_load(f)

    servers = config.get("servers", [])
    if len(servers) < 2:
        pytest.skip("multi_listen.yaml に server が2つ未満のためスキップ")

    listen_count = 0
    for entry in servers:
        server = entry.get("server", {})
        listen_count += len(server.get("listens", []))
    if listen_count == 0:
        pytest.skip("multi_listen.yaml に listen が無いためスキップ")

    ports = []
    while len(ports) < listen_count:
        port = find_free_port()
        if port not in ports:
            ports.append(port)

    port_index = 0
    server_ports = []
    for entry in servers:
        server = entry.get("server", {})
        listens = server.get("listens", [])
        if not listens:
            server_ports.append(None)
            continue
        first_port = None
        for listen_entry in listens:
            listen = listen_entry.get("listen", {})
            listen["interface"] = DEFAULT_HOST
            listen["port"] = ports[port_index]
            if first_port is None:
                first_port = ports[port_index]
            port_index += 1
        server_ports.append(first_port)

    out_path = Path(temp_dir) / "multi_listen_tmp.yaml"
    with open(out_path, "w") as f:
        yaml.safe_dump(config, f, default_flow_style=False, sort_keys=False)

    return out_path, server_ports


class TestValidStartup:
    """正常な起動、基本ケース"""

    @pytest.fixture
    def webserv_bin(self):
        """webservバイナリのパスを返す"""
        test_dir = Path(__file__).parent.parent.parent
        project_root = test_dir.parent
        return str(project_root / "webserv")

    def test_without_config_file(self, webserv_bin):
        proc = subprocess.Popen(
            [webserv_bin],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
        )

        time.sleep(0.5)

        if proc.poll() is None:
            proc.terminate()
            proc.wait()
            assert True, "起動に成功"
        else:
            assert False, "起動に失敗"


    def test_valid_config(self, webserv_bin):
        """有効な設定ファイルは正常に起動できる"""
        test_dir = Path(__file__).parent.parent.parent
        valid_dir = test_dir / "confs" / "valid"
        yaml_files = list(valid_dir.glob("*.yaml"))
        if not yaml_files:
            pytest.skip(f"有効な設定ファイルが見つかりません: {valid_dir}")
        for yaml_file in yaml_files:
            proc = subprocess.Popen(
                [webserv_bin, str(yaml_file)],
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True,
            )
            time.sleep(0.5)
            if proc.poll() is None:
                proc.terminate()
                proc.wait()
            else:
                stdout, stderr = proc.communicate()
                pytest.fail(f"有効な設定ファイル {yaml_file.name} が起動に失敗: {stderr}")

    def test_multi_listen(self, webserv_bin):
        test_dir = Path(__file__).parent.parent.parent
        config_src = test_dir / "confs" / "valid" / "multi_listen.yaml"
        if not config_src.exists():
            pytest.skip("multi_listen.yaml が見つかりません")

        with tempfile.TemporaryDirectory(prefix="multi_listen_test_") as temp_dir:
            temp_config, server_ports = write_multi_listen_config(
                config_src, temp_dir
            )

            if len(server_ports) < 2 or server_ports[0] is None or server_ports[1] is None:
                pytest.skip("multi_listen.yaml から有効なポートが取得できません")

            proc = subprocess.Popen(
                [webserv_bin, str(temp_config)],
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True,
            )

            try:
                ready_a = wait_for_port(DEFAULT_HOST, server_ports[0])
                ready_b = wait_for_port(DEFAULT_HOST, server_ports[1])
                if not (ready_a and ready_b):
                    stdout, stderr = proc.communicate(timeout=1)
                    pytest.fail(
                        "Server did not listen on both ports.\n"
                        f"stdout:\n{stdout}\n"
                        f"stderr:\n{stderr}\n"
                    )

                url = f"http://{DEFAULT_HOST}:{server_ports[0]}/"
                response = requests.get(url, timeout=2)
                assert response.status_code == 200

                url = f"http://{DEFAULT_HOST}:{server_ports[1]}/"
                response = requests.get(url, timeout=2)
                assert response.status_code == 200
            finally:
                if proc.poll() is None:
                    proc.terminate()
                    proc.wait()
