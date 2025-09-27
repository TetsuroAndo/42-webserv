"""clean_comments.py – 42‑friendly C comment manager (v1.2)
================================================================

機能
~~~~
* **デフォルト**: コメントの位置を列挙するだけ。
* **--remove**: インタラクティブに削除。
  1. 削除候補を表示。
  2. 事前に git コミットするか確認。
  3. 実際に削除するか確認。

削除ポリシー
~~~~~~~~~~~~
* ファイル冒頭 1〜11 行は “42 ヘッダー” として検出対象外。
* 範囲 ``/* … */`` / ``// …`` を検出。
* **行全体がコメントのみ** の場合 → その行ごと削除し行を詰める。
* **コード + コメント** の場合 → コメントと直前の空白を削除し、行末に余計な空白を残さない。

制限
~~~~
正規表現ベースであり、文字列リテラル中の ``//`` などは誤検知します。
通常の 42 コードベースを想定。
"""
from __future__ import annotations
import argparse
import re
import subprocess
import sys
from collections import defaultdict
from pathlib import Path
from typing import List, Tuple

# ───────────────────────── constants ─────────────────────────
COMMENT_RE = re.compile(r"//.*?$|/\*.*?\*/", re.DOTALL | re.MULTILINE)
HEADER_BANNER_RE = re.compile(r"\A\s*/\*\s*\*{10,}.*?\*/", re.DOTALL)
SKIP_HEAD_LINES = 11  # always skip first N lines


# ───────────────────────── dataclass ─────────────────────────
class CommentOcc:
    def __init__(self, file: Path, start: int, end: int):
        self.file = file
        self.start = start
        self.end = end
        self._pos: Tuple[int, int] | None = None

    @property
    def line_col(self) -> Tuple[int, int]:
        if self._pos is None:
            data = self.file.read_text(encoding="utf-8", errors="ignore")
            before = data[: self.start]
            line = before.count("\n") + 1
            col = len(before.rsplit("\n", 1)[-1]) + 1
            self._pos = (line, col)
        return self._pos

    def __str__(self) -> str:
        l, c = self.line_col
        return f"{self.file}:{l}:{c}"


# ──────────────────────── helpers ───────────────────────────

def first_n_lines_offset(txt: str, n: int) -> int:
    pos = 0
    for _ in range(n):
        nl = txt.find("\n", pos)
        if nl == -1:
            return len(txt)
        pos = nl + 1
    return pos


def protected_offset(txt: str) -> int:
    o1 = first_n_lines_offset(txt, SKIP_HEAD_LINES)
    m = HEADER_BANNER_RE.match(txt)
    o2 = m.end() if m else 0
    return max(o1, o2)


def source_files() -> List[Path]:
    return [p for p in Path.cwd().rglob("*.[ch]") if p.is_file()]


def scan(path: Path) -> List[CommentOcc]:
    try:
        data = path.read_text(encoding="utf-8", errors="ignore")
    except Exception as e:
        print(f"[warn] cannot read {path}: {e}", file=sys.stderr)
        return []
    off = protected_offset(data)
    body = data[off:]
    occs = [CommentOcc(path, off + m.start(), off + m.end()) for m in COMMENT_RE.finditer(body)]
    return occs


def gather() -> List[CommentOcc]:
    res: List[CommentOcc] = []
    for f in source_files():
        res.extend(scan(f))
    return sorted(res, key=lambda o: (o.file, *o.line_col))


# ──────────────────── mutation utils ────────────────────────

def yes_no(q: str) -> bool:
    while True:
        a = input(q + " [Y/N]: ").strip().lower()
        if a in {"y", "yes"}: return True
        if a in {"n", "no"}: return False
        print("Please answer Y or N.")


def safety_commit():
    if not Path(".git").exists():
        print("[info] not a git repo")
        return
    if yes_no("Create safety git commit before deleting comments?"):
        subprocess.run(["git", "add", "-u"], check=True)
        subprocess.run(["git", "commit", "-m", "backup: before stripping comments"], check=True)
        print("[git] committed backup")


def remove_in_file(path: Path, occs: List[CommentOcc]):
    txt = path.read_text(encoding="utf-8", errors="ignore")
    # work backwards
    for o in sorted(occs, key=lambda x: x.start, reverse=True):
        prev_nl = txt.rfind("\n", 0, o.start) + 1  # 0 if not found
        next_nl_idx = txt.find("\n", o.end)
        next_nl = next_nl_idx + 1 if next_nl_idx != -1 else len(txt)
        before = txt[prev_nl:o.start]
        after = txt[o.end:next_nl_idx if next_nl_idx != -1 else len(txt)]
        if before.strip() == "" and after.strip() == "":
            # line-only comment -> drop entire line (including newline)
            txt = txt[:prev_nl] + txt[next_nl:]
        else:
            # inline comment -> drop preceding spaces + comment, keep newline
            trim_start = o.start
            while trim_start > prev_nl and txt[trim_start - 1] in " \t":
                trim_start -= 1
            txt = txt[:trim_start] + txt[o.end:]
    # cleanup: strip trailing spaces each line
    txt = "\n".join(line.rstrip() for line in txt.splitlines()) + ("\n" if txt.endswith("\n") else "")
    path.write_text(txt, encoding="utf-8")
    print(f"[fix] {path}: {len(occs)} comment(s) removed")


def remove_comments(occs: List[CommentOcc]):
    grouped: defaultdict[Path, List[CommentOcc]] = defaultdict(list)
    for o in occs:
        grouped[o.file].append(o)
    for f, olist in grouped.items():
        remove_in_file(f, olist)


# ───────────────────────── CLI ──────────────────────────────

def cli() -> argparse.ArgumentParser:
    p = argparse.ArgumentParser(description="Scan / remove C comments (42 safe)")
    p.add_argument("--remove", "-r", action="store_true", help="delete comments interactively")
    return p


def main(argv: List[str] | None = None):
    args = cli().parse_args(argv)
    occs = gather()
    if not occs:
        print("✅  no comments found")
        return
    print("Found comments:")
    for o in occs:
        l, c = o.line_col
        snippet = (Path(o.file).read_text(encoding="utf-8", errors="ignore")[o.start:o.end]
        .split("\n", 1)[0][:40])
        print(f"  {o} → {snippet!r}")
    if not args.remove:
        print("Run again with --remove to delete them.")
        return
    safety_commit()
    if not yes_no("Proceed to delete ALL above comments?"):
        print("Aborted.")
        return
    remove_comments(occs)
    print("🎉  done")


if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        print("\n[abort] interrupted")
        sys.exit(130)
