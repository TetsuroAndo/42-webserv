from pathlib import Path

import pytest


@pytest.fixture(scope="package", autouse=True)
def cleanup_uploads_after_test_50():
    """
    After the test_50_timeconnections package finishes, remove any uploaded
    files under test_suite/test_www/uploads except the sentinel .gitignore.
    """
    uploads_dir = Path(__file__).resolve().parents[2] / "test_www" / "uploads"
    yield
    if uploads_dir.exists():
        for entry in uploads_dir.iterdir():
            if entry.name == ".gitignore":
                continue
            if entry.is_file():
                try:
                    entry.unlink()
                except OSError:
                    # Best-effort cleanup; ignore files we cannot remove
                    pass
