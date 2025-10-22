#!/bin/bash

set -e

# This is a COMPREHENSIVE test for the no-argument startup case.
# It checks if the server, running on the default/hardcoded config,
# can correctly handle various success and error cases for GET, POST, and DELETE.

# --- Config ---
GREEN=$(printf '\033[0;32m')
RED=$(printf '\033[0;31m')
BLUE=$(printf '\033[0;34m')
NC=$(printf '\033[0m')

PROJECT_ROOT=$(pwd)
WEBSERV_EXEC="$PROJECT_ROOT/webserv"

# Test settings are based on the default.yaml config
PORT=8080
ADDRESS="127.0.0.1:$PORT"

# Directories and files for testing
ROOT_DIR="/tmp/www"
NO_AUTOINDEX_DIR="$ROOT_DIR/no_autoindex_dir"
UPLOAD_DIR="/tmp/uploads"
NO_PERMS_DIR="/tmp/no_perms_default"

GET_FILE="$ROOT_DIR/index.html"
FORBIDDEN_FILE="$NO_PERMS_DIR/secret.txt"
UPLOAD_SRC_FILE="/tmp/no_args_comprehensive_upload.txt"
UPLOAD_DST_FILE="$UPLOAD_DIR/uploaded_file.bin"

# --- Helper Functions ---

function setup_test_env() {
    echo -e "${BLUE}Setting up comprehensive test environment...${NC}"
    mkdir -p "$ROOT_DIR"
    mkdir -p "$NO_AUTOINDEX_DIR"
    mkdir -p "$UPLOAD_DIR"
    mkdir -p "$NO_PERMS_DIR"

    echo "Default index.html" > "$GET_FILE"
    echo "secret content" > "$FORBIDDEN_FILE"
    chmod 000 "$FORBIDDEN_FILE"
    echo "This is a file to be uploaded for the comprehensive test." > "$UPLOAD_SRC_FILE"

    echo "Building webserv..."
    make > /dev/null
}

function cleanup() {
    echo -e "\n${BLUE}Cleaning up...${NC}"
    chmod 755 "$FORBIDDEN_FILE" || true
    if [ ! -z "$WEBSERV_PID" ]; then
        if kill -0 $WEBSERV_PID 2>/dev/null; then
            kill $WEBSERV_PID
            wait $WEBSERV_PID 2>/dev/null
        fi
    fi
    rm -rf "$ROOT_DIR"
    rm -rf "$UPLOAD_DIR"
    rm -rf "$NO_PERMS_DIR"
    rm -f "$UPLOAD_SRC_FILE"
}

trap cleanup EXIT

# --- Main Test Logic ---

setup_test_env

echo "Starting webserv with no arguments..."
./webserv & 
WEBSERV_PID=$!
sleep 2

if ! kill -0 $WEBSERV_PID 2>/dev/null || ! ss -lnt | grep -q ":$PORT"; then
    echo -e "${RED}FAIL: Server did not start or is not listening on port $PORT.${NC}"
    exit 1
fi
echo -e "${GREEN}OK: Server started and is listening on port $PORT.${NC}"

# ===================
# === GET Tests ===
# ===================
echo -e "\n${BLUE}--- Testing GET... ---${NC}"

# Test: GET 200 OK
HTTP_STATUS=$(curl -s -o /dev/null -w '%{http_code}' "http://$ADDRESS/index.html")
if [[ "$HTTP_STATUS" -ne 200 ]]; then echo -e "${RED}[GET 200] FAIL${NC}"; exit 1; fi
echo -e "${GREEN}[GET 200] OK${NC}"

# Test: GET 404 Not Found
HTTP_STATUS=$(curl -s -o /dev/null -w '%{http_code}' "http://$ADDRESS/non_existent_file.txt")
if [[ "$HTTP_STATUS" -ne 404 ]]; then echo -e "${RED}[GET 404] FAIL${NC}"; exit 1; fi
echo -e "${GREEN}[GET 404] OK${NC}"

# Test: GET 403 Forbidden (File Permissions)
HTTP_STATUS=$(curl -s -o /dev/null -w '%{http_code}' "http://$ADDRESS/forbidden/secret.txt")
if [[ "$HTTP_STATUS" -ne 403 ]]; then echo -e "${RED}[GET 403] FAIL${NC}"; exit 1; fi
echo -e "${GREEN}[GET 403] OK${NC}"

# Test: GET Directory with autoindex on
RESPONSE_BODY=$(curl -s "http://$ADDRESS/")
if ! echo "$RESPONSE_BODY" | grep -q "index.html"; then echo -e "${RED}[GET autoindex] FAIL${NC}"; exit 1; fi
echo -e "${GREEN}[GET autoindex] OK${NC}"

# Test: GET Directory with autoindex off
HTTP_STATUS=$(curl -s -o /dev/null -w '%{http_code}' "http://$ADDRESS/no_autoindex/")
if [[ "$HTTP_STATUS" -ne 403 ]]; then echo -e "${RED}[GET no autoindex] FAIL${NC}"; exit 1; fi
echo -e "${GREEN}[GET no autoindex] OK${NC}"

# ====================
# === POST Tests ===
# ====================
echo -e "\n${BLUE}--- Testing POST... ---${NC}"

# Test: POST 201 Created (Upload)
HTTP_STATUS=$(curl -s -o /dev/null -w '%{http_code}' -X POST --data-binary "@$UPLOAD_SRC_FILE" "http://$ADDRESS/upload/ignored_filename")
if [[ "$HTTP_STATUS" -ne 201 || ! -f "$UPLOAD_DST_FILE" ]]; then echo -e "${RED}[POST 201] FAIL${NC}"; exit 1; fi
echo -e "${GREEN}[POST 201] OK${NC}"

# Test: POST 405 Method Not Allowed
HTTP_STATUS=$(curl -s -o /dev/null -w '%{http_code}' -X POST --data "test" "http://$ADDRESS/get_only/hello.txt")
if [[ "$HTTP_STATUS" -ne 405 ]]; then echo -e "${RED}[POST 405] FAIL${NC}"; exit 1; fi
echo -e "${GREEN}[POST 405] OK${NC}"

# =====================
# === DELETE Tests ===
# =====================
echo -e "\n${BLUE}--- Testing DELETE... ---${NC}"

# Test: DELETE 204 No Content
HTTP_STATUS=$(curl -s -o /dev/null -w '%{http_code}' -X DELETE "http://$ADDRESS/upload/uploaded_file.bin")
if [[ "$HTTP_STATUS" -ne 204 || -f "$UPLOAD_DST_FILE" ]]; then echo -e "${RED}[DELETE 204] FAIL${NC}"; exit 1; fi
echo -e "${GREEN}[DELETE 204] OK${NC}"

# Test: DELETE 404 Not Found
HTTP_STATUS=$(curl -s -o /dev/null -w '%{http_code}' -X DELETE "http://$ADDRESS/upload/non_existent_file")
if [[ "$HTTP_STATUS" -ne 404 ]]; then echo -e "${RED}[DELETE 404] FAIL${NC}"; exit 1; fi
echo -e "${GREEN}[DELETE 404] OK${NC}"

# Test: DELETE 405 Method Not Allowed
HTTP_STATUS=$(curl -s -o /dev/null -w '%{http_code}' -X DELETE "http://$ADDRESS/get_only/hello.txt")
if [[ "$HTTP_STATUS" -ne 405 ]]; then echo -e "${RED}[DELETE 405] FAIL${NC}"; exit 1; fi
echo -e "${GREEN}[DELETE 405] OK${NC}"

echo -e "\n${GREEN}SUCCESS: All tests passed for the comprehensive no-argument startup case.${NC}"
exit 0