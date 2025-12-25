"""
POST メソッドのテスト（雛形）
"""
import pytest
import requests


class TestPOST:
    pass


from pathlib import Path

@pytest.mark.config("valid/post.yaml")
def test_post_created_and_file_saved(managed_server):
    # 最小データを送信
    url = f"{managed_server['base_url']}/upload"
    data = b"hello world\n"
    resp = requests.post(url, data=data, headers={"Content-Type": "text/plain"})
    assert resp.status_code in (200, 201)

    upload_dir = Path("test/test_www/uploads")
    txt_files = list(upload_dir.glob("*.txt"))

    assert txt_files, f"No .txt file found in {upload_dir}"

    saved = txt_files[0]
    assert saved.read_bytes() == data

    saved.unlink()


@pytest.mark.config("valid/post_test.yaml")
def test_post_payload_too_large(managed_server):
    url = f"{managed_server['base_url']}/upload"

    # 2KB データ生成
    body = b"x" * 2048
    resp = requests.post(url, data=body, headers={"Content-Type": "application/octet-stream"})
    assert resp.status_code == 413

    # アップロードディレクトリ確認
    upload_dir = Path("test/www_test/uploads")
    txt_files = list(upload_dir.glob("*.txt"))

    # .txt ファイルが作成されていないことを確認
    assert not txt_files, f"Unexpected .txt file(s) found: {[f.name for f in txt_files]}"

    # 万が一残っていたら削除してクリーンアップ
    for f in txt_files:
        try:
            f.unlink()
        except Exception as e:
            print(f"Warning: failed to delete {f}: {e}")


@pytest.mark.config("valid/config_basic_get.yaml")
def test_post_method_not_allowed(managed_server):
    url = f"{managed_server['base_url']}/"
    resp = requests.post(url, data=b"x", headers={"Content-Type": "text/plain"})
    assert resp.status_code == 405


@pytest.mark.config("valid/post_test.yaml")
def test_post_no_ex_dir(managed_server):
    url = f"{managed_server['base_url']}/EX-upload"
    # 存在しないディレクトリへの POST は 404 を期待
    resp = requests.post(url, data=b"hello", headers={"Content-Type": "text/plain"})
    assert resp.status_code == 404

    # 念のため、アップロードディレクトリにファイルが作られていないことも確認
    upload_dir = Path("test/www_test/uploads")
    txt_files = list(upload_dir.glob("*.txt"))
    assert not txt_files, f"Unexpected .txt file(s) found: {[f.name for f in txt_files]}"
@pytest.mark.config("valid/post_test.yaml")
def test_not_allow_file_type(managed_server):
    url = f"{managed_server['base_url']}/"
    # 対応していないcontent typeの POST は 403 を期待
    resp = requests.post(url, data=b"hello", headers={"Content-Type": "hoge/huga"})
    assert resp.status_code == 403

    # 念のため、アップロードディレクトリにファイルが作られていないことも確認
    upload_dir = Path("test/www_test/uploads")
    txt_files = list(upload_dir.glob("*.txt"))
    assert not txt_files, f"Unexpected .txt file(s) found: {[f.name for f in txt_files]}"

@pytest.mark.config("valid/post.yaml")
def test_post_no_permission_directory(managed_server):
    base = managed_server['base_url']

    here = Path(__file__).resolve().parent
    # POST 実行
    url_post = f"{base}/upload/"
    upload_dir = here / "../../test_www/uploads/"

    # ディレクトリの w 権限を抜く
    old_mode = upload_dir.stat().st_mode
    upload_dir.chmod(0o555)  # 7-2=5

    try:
        # DELETE 実行
        resp = requests.post(url_post, "hello world",headers={"Content-Type": "text/plain"})
        # 評価 禁止 or 見えない
        assert resp.status_code in (403, 404)

    finally:
        # 後始末：権限を戻す
        upload_dir.chmod(old_mode)
        upload_dir = Path("test/www_test/uploads")
        txt_files = list(upload_dir.glob("*.txt"))
        assert not txt_files, f"Unexpected .txt file(s) found: {[f.name for f in txt_files]}"