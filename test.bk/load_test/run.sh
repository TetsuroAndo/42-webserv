#!/usr/bin/env bash
set -euo pipefail

GREEN='\033[0;32m'
RED='\033[0;31m'
BLUE='\033[0;34m'
NC='\033[0m'

ROOT_DIR="$(cd "$(dirname "$0")/../.." && pwd)"
cd "$ROOT_DIR"

CONFIG_PATH="${1:-test/load_test/config.yaml}"
PORT=8082
ADDR="127.0.0.1:${PORT}"
WEBSERV_EXEC="${ROOT_DIR}/webserv"

cleanup() {
  echo -e "${BLUE}Cleaning up...${NC}"
  if [[ -n "${WEBSERV_PID:-}" ]]; then
    kill "$WEBSERV_PID" 2>/dev/null || true
    wait "$WEBSERV_PID" 2>/dev/null || true
  fi
}
trap cleanup EXIT

echo -e "${BLUE}Building webserv...${NC}"
make -s

echo -e "${BLUE}Starting webserv...${NC}"
"$WEBSERV_EXEC" "$CONFIG_PATH" >/dev/null &
WEBSERV_PID=$!
sleep 1
if ! kill -0 "$WEBSERV_PID" 2>/dev/null; then
  echo -e "${RED}Server failed to start${NC}"
  exit 1
fi
echo -e "Server PID: ${WEBSERV_PID}, Port: ${PORT}\n"

PY="python3"
LOAD="test/load_test/load.py"

echo -e "${BLUE}Scenario 1: GET simple CGI (5k req, c=50)${NC}"
$PY "$LOAD" --url "http://${ADDR}/cgi-bin/simple.py" --concurrency 50 --requests 5000 || true

echo -e "\n${BLUE}Scenario 2: POST echo CGI (2k req, c=50)${NC}"
$PY "$LOAD" --url "http://${ADDR}/cgi-bin/echo.py" --method POST --data 'hello=world' --concurrency 50 --requests 2000 || true

echo -e "\n${BLUE}Scenario 3: GET echo with query (2k req, c=50)${NC}"
$PY "$LOAD" --url "http://${ADDR}/cgi-bin/echo.py?foo=bar&baz=qux" --concurrency 50 --requests 2000 || true

echo -e "\n${GREEN}Done.${NC}"

