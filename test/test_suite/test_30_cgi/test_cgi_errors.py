import pytest
import requests
import os
import stat
import shutil
import time
from conftest import TEST_WWW_DIR, Webserv

# テスト用のCGIスクリプトとディレクトリのベースパス
CGI_BIN_DIR = os.path.join(TEST_WWW_DIR, "cgi-bin")

# --- Fixtures ---

@pytest.fixture(scope="module")
def server_config():
    """テスト用のコンフィグファイルパスを提供するFixture"""
    return "test/confs/valid/cgi_errors.yaml"

@pytest.fixture(scope="module")
def setup_cgi_environment(server_config):
    """
    動的テストに必要なCGI環境（ファイルのコピー、ディレクトリ）をセットアップし、
    テスト終了時にクリーンアップします。
    """

    # 1. 動的テスト用のインタープリタのコピーを作成
    interp_orig = "/usr/bin/python3"
    interp_copy = os.path.join(CGI_BIN_DIR, "my_python_copy")
    if os.path.exists(interp_orig):
        shutil.copy(interp_orig, interp_copy)
        os.chmod(interp_copy, 0o755) # 実行権限を付与
    else:
        pytest.skip("/usr/bin/python3 が見つかりません。テストをスキップします。")

    # 2. スクリプトがディレクトリの場合のテスト用ディレクトリを作成
    dir_as_script = os.path.join(CGI_BIN_DIR, "directory_as_script.py")
    os.makedirs(dir_as_script, exist_ok=True)

    # 3. 親ディレクトリの権限テスト用ディレクトリとスクリプトを作成
    no_perms_dir = os.path.join(CGI_BIN_DIR, "no_perms_dir")
    inaccessible_script = os.path.join(no_perms_dir, "inaccessible.py")
    os.makedirs(no_perms_dir, exist_ok=True)
    with open(inaccessible_script, "w") as f:
        f.write("#!/usr/bin/python3\n")
        f.write("print('Content-Type: text/plain\\r\\n\\r\\nForbidden')\n")
    os.chmod(inaccessible_script, 0o644)

    # 4. 動的削除テスト用のスクリプトを作成
    dynamic_script_path = os.path.join(CGI_BIN_DIR, "dynamic_script.py")
    with open(dynamic_script_path, "w") as f:
        f.write("#!/usr/bin/python3\n")
        f.write("print('Content-Type: text/plain\\r\\n\\r\\nDynamic OK')\n")
    os.chmod(dynamic_script_path, 0o755)


    # サーバーを起動してテストを実行
    with Webserv(server_config) as server:
        yield server # テスト実行

    # --- Teardown (クリーンアップ) ---
    if os.path.exists(interp_copy):
        os.remove(interp_copy)
    if os.path.exists(dir_as_script):
        shutil.rmtree(dir_as_script)
    if os.path.exists(no_perms_dir):
        # 権限を戻してから削除 (Windowsでは不要だがUNIX系では念のため)
        os.chmod(no_perms_dir, 0o755)
        shutil.rmtree(no_perms_dir)
    if os.path.exists(dynamic_script_path):
        os.remove(dynamic_script_path)


# --- Test Cases ---

def test_cgi_non_existent_interpreter(setup_cgi_environment):
    """
    目的:
    設定ファイルで指定されたCGIインタープリタが存在しない場合、
    CGIワーカーが 500 Internal Server Error を正しく生成するかテストする。

    処理:
    /cgi-no-interp/ (存在しないインタープリタが設定されている) の
    echo.py にGETリクエストを送信する。
    """
    url = "http://127.0.0.1:8090/cgi-no-interp/echo.py"
    response = requests.get(url, timeout=10)

    assert response.status_code == 500
    assert "CGI Error: Interpreter not found or inaccessible" in response.text
    assert response.headers["Content-Type"] == "text/plain"


def test_cgi_non_executable_interpreter(setup_cgi_environment):
    """
    目的:
    設定ファイルで指定されたCGIインタープリタが実行可能ファイルでない場合、
    CGIワーカーが 500 Internal Server Error を正しく生成するかテストする。

    処理:
    /cgi-bad-interp/ (インタープリタとして.txtファイルが指定されている) の
    echo.py にGETリクエストを送信する。
    """
    url = "http://127.0.0.1:8090/cgi-bad-interp/echo.py"
    response = requests.get(url, timeout=10)

    assert response.status_code == 500
    assert "CGI Error: Interpreter is not executable" in response.text
    assert response.headers["Content-Type"] == "text/plain"


def test_cgi_non_existent_script(setup_cgi_environment):
    """
    目的:
    リクエストされたCGIスクリプトファイルが存在しない場合、
    CGIワーカーが 404 Not Found を正しく生成するかテストする。

    処理:
    /cgi-bin/ (正常な設定) の non_existent_script.py (存在しないファイル) に
    GETリクエストを送信する。
    """
    url = "http://127.0.0.1:8090/cgi-bin/non_existent_script.py"
    response = requests.get(url, timeout=10)

    assert response.status_code == 404
    assert "CGI Error: Script not found" in response.text
    assert response.headers["Content-Type"] == "text/plain"


def test_cgi_script_is_directory(setup_cgi_environment):
    """
    目的:
    リクエストされたCGIスクリプトパスがファイルではなくディレクトリである場合、
    CGIワーカーが 403 Forbidden を正しく生成するかテストする。

    処理:
    /cgi-bin/ (正常な設定) の directory_as_script.py (Fixtureでディレクトリとして作成) に
    GETリクエストを送信する。
    """
    url = "http://127.0.0.1:8090/cgi-bin/directory_as_script.py"
    response = requests.get(url, timeout=10)

    assert response.status_code == 403
    assert "CGI Error: Script is not a regular file" in response.text
    assert response.headers["Content-Type"] == "text/plain"


def test_cgi_script_parent_no_exec_permission(setup_cgi_environment):
    """
    目的:
    CGIスクリプト自体は存在するが、その親ディレクトリに検索(x)権限がなく
    stat() が EACCES で失敗する場合、CGIワーカーが 403 Forbidden を生成するかテストする。

    処理:
    1. Fixtureで作成した /cgi-bin/no_perms_dir/ の権限を 0o755 (rwx) から 0o644 (rw-) に変更する。
    2. /cgi-bin/no_perms_dir/inaccessible.py にGETリクエストを送信する。
    3. サーバーが 403 を返すことを確認する。
    4. (重要) 権限を 0o755 に戻し、クリーンアップできるようにする。
    """
    no_perms_dir = os.path.join(CGI_BIN_DIR, "no_perms_dir")
    url = "http://127.0.0.1:8090/cgi-bin/no_perms_dir/inaccessible.py"

    try:
        # 親ディレクトリから実行(検索)権限を削除
        os.chmod(no_perms_dir, 0o644) # r-xr-xr-x -> rw-r--r--

        response = requests.get(url, timeout=10)

        assert response.status_code == 403
        assert "CGI Error: Script access denied" in response.text
        assert response.headers["Content-Type"] == "text/plain"

    finally:
        # クリーンアップのために権限を元に戻す
        os.chmod(no_perms_dir, 0o755)


def test_cgi_dynamic_interpreter_permissions(setup_cgi_environment):
    """
    目的:
    サーバー実行中にCGIインタープリタの実行権限を変更し、
    サーバーがリクエストごとに正しくエラーをハンドリングできるかテストする。

    処理:
    1. /cgi-dyn-interp/ (コピーした my_python_copy を使用) にリクエストを送り、200 OK を確認。
    2. my_python_copy の実行権限を削除 (chmod 0644)。
    3. 再度リクエストを送り、500 (Interpreter is not executable) を確認。
    4. my_python_copy の実行権限を付与 (chmod 0755)。
    5. 再度リクエストを送り、200 OK を確認。
    """
    url = "http://127.0.0.1:8090/cgi-dyn-interp/echo.py"
    interp_copy_path = os.path.join(CGI_BIN_DIR, "my_python_copy")

    # 1. 正常な状態を確認
    response1 = requests.get(url, params={"message": "test1"}, timeout=10)
    assert response1.status_code == 200
    assert "GET message: test1" in response1.text

    try:
        # 2. 実行権限を削除
        os.chmod(interp_copy_path, 0o644) # rwx -> rw-
        time.sleep(0.1) # ファイルシステムの変更が反映されるのを少し待つ

        # 3. エラーになることを確認
        response2 = requests.get(url, params={"message": "test2"}, timeout=10)
        assert response2.status_code == 500
        assert "CGI Error: Interpreter is not executable" in response2.text

        # 4. 実行権限を復元
        os.chmod(interp_copy_path, 0o755) # rw- -> rwx
        time.sleep(0.1)

        # 5. 正常に戻ることを確認
        response3 = requests.get(url, params={"message": "test3"}, timeout=10)
        assert response3.status_code == 200
        assert "GET message: test3" in response3.text

    finally:
        # Fixtureのクリーンアップに任せるが、念のため権限を戻しておく
        if not os.access(interp_copy_path, os.X_OK):
            os.chmod(interp_copy_path, 0o755)


def test_cgi_dynamic_script_deletion(setup_cgi_environment):
    """
    目的:
    サーバー実行中にCGIスクリプトを削除・再作成し、
    サーバーがリクエストごとに正しくファイル存在チェックを行えるかテストする。

    処理:
    1. /cgi-bin/dynamic_script.py にリクエストを送り、200 OK を確認。
    2. dynamic_script.py を削除する。
    3. 再度リクエストを送り、404 (Script not found) を確認。
    4. dynamic_script.py を再作成する。
    5. 再度リクエストを送り、200 OK を確認。
    """
    url = "http://127.0.0.1:8090/cgi-bin/dynamic_script.py"
    script_path = os.path.join(CGI_BIN_DIR, "dynamic_script.py")
    script_content = (
        "#!/usr/bin/python3\n"
        "print('Content-Type: text/plain\\r\\n\\r\\nDynamic OK')\n"
    )

    try:
        # 1. 正常な状態を確認
        assert os.path.exists(script_path)
        response1 = requests.get(url, timeout=10)
        assert response1.status_code == 200
        assert "Dynamic OK" in response1.text

        # 2. スクリプトを削除
        os.remove(script_path)
        time.sleep(0.1)

        # 3. 404 エラーになることを確認
        assert not os.path.exists(script_path)
        response2 = requests.get(url, timeout=10)
        assert response2.status_code == 404
        assert "CGI Error: Script not found" in response2.text

        # 4. スクリプトを再作成
        with open(script_path, "w") as f:
            f.write(script_content)
        os.chmod(script_path, 0o755)
        time.sleep(0.1)

        # 5. 正常に戻ることを確認
        assert os.path.exists(script_path)
        response3 = requests.get(url, timeout=10)
        assert response3.status_code == 200
        assert "Dynamic OK" in response3.text

    finally:
        # Fixtureのクリーンアップに任せる
        pass
