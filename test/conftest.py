import pytest
import subprocess
import os
import signal
import time
import socket
import yaml
import tempfile
import shutil
from contextlib import closing
from pathlib import Path
import psutil


WEBSERV_BIN = Path(__file__).parent.parent / "webserv"  # プロジェクトルートのwebserv
CONFIGS_DIR = Path(__file__).parent / "confs"
DEFAULT_HOST = "127.0.0.1"

# --- ビルド (テストセッション開始時に1回だけ実行) ---
@pytest.fixture(scope="session", autouse=True)
def build_server():
    """テストセッション開始時に 'make' を実行してサーバーをビルドする"""
    print("\nBuilding webserv executable...")
    # プロジェクトルートでmakeを実行
    try:
        subprocess.run(
            ["make"],
            cwd=WEBSERV_BIN.parent,
            check=True,
            capture_output=True,
            text=True,
            timeout=30
        )
        print("Build successful.")
    except subprocess.CalledProcessError as e:
        print(f"Build FAILED:\n{e.stderr}")
        pytest.exit("Webserv build failed, aborting tests.", 1)
    except subprocess.TimeoutExpired:
        pytest.exit("Webserv build timed out.", 1)

    if not WEBSERV_BIN.exists():
        pytest.exit(f"Webserv executable not found at {WEBSERV_BIN}", 1)

# --- ヘルパー関数 ---
def find_free_port():
    """OSが利用可能なポートを動的に見つける"""
    with closing(socket.socket(socket.AF_INET, socket.SOCK_STREAM)) as s:
        s.bind((DEFAULT_HOST, 0))
        s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        return s.getsockname()[1]


def parse_config_port(config_path):
    """YAML設定ファイルからポート番号を抽出する"""
    try:
        with open(config_path, 'r') as f:
            config = yaml.safe_load(f)

        # 設定ファイル構造: servers -> server -> listens -> listen -> port
        servers = config.get('servers', [])
        if not servers:
            return None

        server = servers[0].get('server', {})
        listens = server.get('listens', [])
        if not listens:
            return None

        listen = listens[0].get('listen', {})
        port = listen.get('port', None)
        return port if port else None

    except Exception as e:
        print(f"Warning: Could not parse port from config: {e}")
        return None


def create_temp_config_with_port(original_config, new_port):
    """元の設定ファイルを読み込み、ポートを置換した一時ファイルを作成"""
    with open(original_config, 'r') as f:
        config = yaml.safe_load(f)

    # ポートを更新
    config['servers'][0]['server']['listens'][0]['listen']['port'] = new_port

    # 一時ファイルを作成
    temp_fd, temp_path = tempfile.mkstemp(suffix='.yaml', prefix='test_config_')

    try:
        with os.fdopen(temp_fd, 'w') as f:
            yaml.dump(config, f)
        return temp_path
    except Exception:
        os.close(temp_fd)
        raise


# --- サーバー管理Fixture ---
@pytest.fixture
def managed_server(request):
    """
    マーカーで指定された設定ファイルを使ってwebservを起動し、
    テスト終了後に確実に停止させるフィクスチャ。

    使用法:
    @pytest.mark.config("valid/config_basic_get.yaml")
    def test_something(managed_server):
        response = requests.get(f"{managed_server['base_url']}/path")
        assert response.status_code == 200
    """
    proc = None
    temp_config_path = None

    # マーカーから設定ファイル名を取得
    marker = request.node.get_closest_marker("config")
    if not marker:
        pytest.fail("managed_serverフィクスチャを使用するには@pytest.mark.configマーカーが必要です。")

    config_name = marker.args[0] if marker.args else None
    if not config_name:
        pytest.fail("configマーカーには設定ファイル名を指定してください。")

    # 設定ファイルのパスを解決 (valid/invalidの両方をサポート)
    config_path = CONFIGS_DIR / config_name
    if not config_path.exists():
        # サブディレクトリで試す
        for subdir in ["valid", "invalid"]:
            candidate = CONFIGS_DIR / subdir / config_name
            if candidate.exists():
                config_path = candidate
                break
        else:
            pytest.fail(f"Config file not found: {CONFIGS_DIR} (searched for {config_name})")

    try:
        # 元の設定ファイルからポートを取得
        original_port = parse_config_port(config_path)

        # テスト用に一時的にポートを動的に割り当てる
        # (このプロジェクトでは8080などの固定ポートを使っているようなので、
        #  そのまま使用する)
        test_port = original_port if original_port else 8080

        # ポートを動的に置換した一時設定ファイルを作成
        temp_config_path = create_temp_config_with_port(config_path, test_port)

        # サーバー起動
        cmd = [str(WEBSERV_BIN), temp_config_path]

        try:
            proc = subprocess.Popen(
                cmd,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True,
                preexec_fn=os.setsid
            )
        except Exception as e:
            pytest.fail(f"Failed to start server process: {e}")

        # サーバーが起動してポートをリッスンするのを待つ
        max_wait = 5
        start_time = time.time()
        server_started = False

        while time.time() - start_time < max_wait:
            if proc.poll() is not None:
                # サーバーが異常終了した
                stdout, stderr = proc.communicate()
                pytest.fail(
                    f"Server exited unexpectedly (code: {proc.returncode})\n"
                    f"STDOUT:\n{stdout}\n"
                    f"STDERR:\n{stderr}"
                )

            try:
                with socket.create_connection((DEFAULT_HOST, test_port), timeout=0.1):
                    server_started = True
                    break
            except (ConnectionRefusedError, socket.timeout):
                time.sleep(0.1)

        if not server_started:
            stdout, stderr = proc.communicate()
            proc.kill()
            pytest.fail(
                f"Server failed to start and listen on port {test_port} within {max_wait}s.\n"
                f"STDOUT:\n{stdout}\n"
                f"STDERR:\n{stderr}"
            )

        # テストにサーバー情報を渡す
        base_url = f"http://{DEFAULT_HOST}:{test_port}"
        yield {
            "process": proc,
            "base_url": base_url,
            "port": test_port,
            "config_path": config_path
        }

    finally:
        # テスト終了後のクリーンアップ
        if proc:
            print(f"\nShutting down server (PID: {proc.pid})...")
            try:
                # プロセスグループごとkill
                try:
                    parent = psutil.Process(proc.pid)
                    for child in parent.children(recursive=True):
                        child.kill()
                    parent.kill()
                except psutil.NoSuchProcess:
                    pass
            except (OSError, ProcessLookupError):
                # psutilが使えない場合はプロセスグループをkill
                try:
                    os.killpg(os.getpgid(proc.pid), signal.SIGKILL)
                except (OSError, ProcessLookupError):
                    pass

            # プロセスが終了するのを待つ
            try:
                proc.wait(timeout=2)
            except subprocess.TimeoutExpired:
                proc.kill()
                proc.wait()

            # ログ出力
            stdout, stderr = proc.communicate()
            if proc.returncode != 0 and proc.returncode != -signal.SIGKILL:
                print(f"Server exited with code: {proc.returncode}")
                if stderr:
                    print(f"STDERR:\n{stderr[:500]}")  # 最初の500文字だけ表示

        # 一時設定ファイルを削除
        if temp_config_path and os.path.exists(temp_config_path):
            try:
                os.unlink(temp_config_path)
            except Exception:
                pass
