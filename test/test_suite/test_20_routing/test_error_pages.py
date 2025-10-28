"""
エラーページのテスト（雛形）
"""
import pytest
import requests


class TestErrorPages:
    @pytest.mark.skip("カスタム404用の設定ファイル追加後に有効化")
    def test_custom_404(self, managed_server):
        r = requests.get(f"{managed_server['base_url']}/no_such_path")
        assert r.status_code == 404
        assert "Custom 404" in r.text

    @pytest.mark.config("valid/config_basic_get.yaml")
    def test_default_404(self, managed_server):
        r = requests.get(f"{managed_server['base_url']}/no_such_path")
        assert r.status_code == 404
        # 具体的な本文は実装依存のため、最低限コードのみを検証
