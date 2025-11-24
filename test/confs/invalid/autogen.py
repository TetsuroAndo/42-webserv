#!/usr/bin/env python3
import yaml
import os

CONFIG = "server.yaml"
BASE = "server"
LOC = os.path.join(BASE, "location")

TEMPLATE = """servers:
  - server:
      listens:
        - listen:
            interface: 0.0.0.0
            port: 8080

      error_logs:
        - error_log:
            sink: file
            level: debug
        - error_log:
            sink: console
            level: debug
"""

def write_invalid(dirpath, filename):
    os.makedirs(dirpath, exist_ok=True)
    filepath = os.path.join(dirpath, filename)

    # ⭐ 上書き防止：ファイルが存在する場合はスキップ
    if os.path.exists(filepath):
        print("スキップ（既存のため）:", filepath)
        return

    with open(filepath, "w") as f:
        f.write(TEMPLATE)

    print("生成:", filepath)


# YAML 読み込み
with open(CONFIG, "r") as f:
    data = yaml.safe_load(f)

server = data["servers"][0]["server"]

# ---- server 直下処理 ----
for key, value in server.items():

    # ---- スカラー値 ----
    if isinstance(value, (str, int, float, bool)):
        write_invalid(os.path.join(BASE, key), f"invalid_{key}.yaml")

    # ---- map（interpreterPath, error_pages など）----
    elif isinstance(value, dict):
        for child in value.keys():
            clean = str(child).lstrip(".")
            write_invalid(os.path.join(BASE, key), f"invalid_{clean}.yaml")

    # ---- array（listens, access_logs, error_logs, locations）----
    elif isinstance(value, list):
        for elem in value:
            if isinstance(elem, dict):
                for child in elem.keys():
                    clean = str(child).lstrip(".")
                    write_invalid(os.path.join(BASE, key), f"invalid_{clean}.yaml")


# ---- locations/location ----
locations = server.get("locations", [])
loc_keys = set()

for loc in locations:
    if isinstance(loc, dict) and "location" in loc:
        loc_keys |= set(loc["location"].keys())

for lk in loc_keys:
    clean = str(lk).lstrip(".")
    write_invalid(LOC, f"invalid_{clean}.yaml")

print("=== 完了しました ===")
