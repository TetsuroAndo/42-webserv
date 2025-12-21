from pathlib import Path

# Clean up uploaded files from previous test runs, but keep the sentinel .gitignore
uploads_dir = Path(__file__).resolve().parents[2] / "test_www" / "uploads"
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
