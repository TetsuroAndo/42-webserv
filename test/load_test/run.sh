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

# Check for zombie processes left by webserv (children with state Z)
check_zombies() {
  echo -e "${BLUE}Checking for zombie child processes...${NC}"
  # Give a brief grace period for the server to reap children
  sleep 1
  # ps output: pid ppid state command; filter by PPID == WEBSERV_PID and state contains Z
  if ps -o pid=,ppid=,stat=,comm= -ax >/dev/null 2>&1; then
    ZOMBIES=$(ps -o pid=,ppid=,stat=,comm= -ax | awk -v ppid="$WEBSERV_PID" '$2==ppid && $3 ~ /Z/')
  else
    # Fallback for minimal ps implementations
    ZOMBIES=$(ps -A -o pid=,ppid=,stat=,comm= | awk -v ppid="$WEBSERV_PID" '$2==ppid && $3 ~ /Z/')
  fi

  if [[ -n "$ZOMBIES" ]]; then
    echo -e "${RED}Zombie processes detected (PID PPID STAT COMM):${NC}"
    echo "$ZOMBIES"
    return 1
  else
    echo -e "${GREEN}No zombie child processes detected.${NC}"
    return 0
  fi
}

if ! check_zombies; then
  # Non-zero exit to signal failure; cleanup trap will still run
  exit 1
fi
