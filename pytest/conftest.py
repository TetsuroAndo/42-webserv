import pytest
import subprocess
import os
import signal
import time
import socket
from contextlib import closing
from pathlib import Path
import psutil


WEBSERV_BIN = Path(__file__).parent / "webserv"
CONFIGS_DIR = Path(__file__).parent / "config"
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

# --- 動的に空きポートを見つけるヘルパー ---
def find_free_port():
    """OSが利用可能なポートを動的に見つける"""
    with closing(socket.socket(socket.AF_INET, socket.SOCK_STREAM)) as s:
        s.bind((DEFAULT_HOST, 0)) # ポート0を指定するとOSが空きポートを割り当てる
        s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        return s.getsockname()[1] # (host, port) のうちportを返す

# --- サーバー管理Fixture (テスト毎に実行) ---
@pytest.fixture
def managed_server():
    """
    サーバープロセスを起動・監視し、テスト終了時に確実に停止する
    'yield' を使って、テスト実行中にサーバーを稼働させ続ける
    """
    proc = None
    config_path = None
    port = None
    
    def _start_server(config_name, **kwargs):
        nonlocal proc, config_path, port

        # 1. 設定ファイルのパスを解決
        config_path = CONFIGS_DIR / "valid" / config_name
        if not config_path.exists():
            pytest.fail(f"Config file not found: {config_path}")

        # 2. 空きポートを取得 (ポートをハードコーディングしないためのベストプラクティス)
        port = find_free_port()
        
        # 3. (オプション) 設定ファイル内のポートを動的に置換
        #    ここでは単純化のため、設定ファイルが 8080 を使う前提とする
        #    より高度な実装: configを読み込み、ポート番号を {port} のような
        #    プレースホルダにして、ここで置換した一時ファイルを使う
        
        # 4. サーバー起動
        #    os.setsid() は子プロセス (CGIなど) も含めて
        #    グループキルできるようにするために重要
        cmd = [str(WEBSERV_BIN), str(config_path)]
        
        try:
            proc = subprocess.Popen(
                cmd,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True,
                preexec_fn=os.setsid # プロセスグループを作成
            )
        except Exception as e:
            pytest.fail(f"Failed to start server process: {e}")

        # 5. サーバーが起動してポートをリッスンするのを待つ
        max_wait = 5  # 最大5秒待つ
        start_time = time.time()
        while time.time() - start_time < max_wait:
            try:
                with socket.create_connection((DEFAULT_HOST, port), timeout=0.1):
                    break # 接続できたら起動成功
            except (ConnectionRefusedError, socket.timeout):
                time.sleep(0.1)
        else:
            # タイムアウトした場合
            stdout, stderr = proc.communicate()
            proc.kill()
            pytest.fail(
                f"Server failed to start and listen on port {port} within {max_wait}s.\n"
                f"STDOUT:\n{stdout}\n"
                f"STDERR:\n{stderr}"
            )
            
        # 6. テスト本体にサーバーのURLを渡す
        base_url = f"http://{DEFAULT_HOST}:{port}"
        return {"process": proc, "base_url": base_url, "port": port}

    # --- ここでテストが実行される ---
    yield _start_server # _start_server関数をテストに渡す

    # --- テスト終了後のクリーンアップ ---
    if proc:
        print(f"\nShutting down server (PID: {proc.pid})...")
        try:
            # psutil を使って子プロセスごとkillする (CGIが残るのを防ぐ)
            parent = psutil.Process(proc.pid)
            for child in parent.children(recursive=True):
                child.kill()
            parent.kill()
        except psutil.NoSuchProcess:
            pass # すでに終了している
        except NameError:
            # psutilがない場合: プロセスグループをkill
            try:
                os.killpg(os.getpgid(proc.pid), signal.SIGKILL)
            except ProcessLookupError:
                pass # すでに終了している

        # プロセスが終了するのを待つ (ゾンビプロセス化を防ぐ)
        try:
            proc.wait(timeout=2)
        except subprocess.TimeoutExpired:
            print(f"Server (PID: {proc.pid}) did not terminate gracefully, forcing kill.")
            proc.kill()
            proc.wait()
        
        stdout, stderr = proc.communicate()
        if proc.returncode != 0 and proc.returncode != -signal.SIGKILL:
            print(f"Server exited with non-zero code: {proc.returncode}")
            print(f"STDERR:\n{stderr}")
