#!/bin/bash

set -e

# --- Config ---
# Color definitions using printf for better portability
GREEN=$(printf '\033[0;32m')
RED=$(printf '\033[0;31m')
BLUE=$(printf '\033[0;34m')
NC=$(printf '\033[0m')

cd "$(dirname "$0")/../.."
PROJECT_ROOT=$(pwd)

WEBSERV_EXEC="$PROJECT_ROOT/webserv"
TEST_CONF="$PROJECT_ROOT/test/http_methods_test/test.yaml"

# Test directories and files
GET_ROOT="$PROJECT_ROOT/www/http_test_root"
NO_AUTOINDEX_DIR="$GET_ROOT/no_autoindex_dir"
UPLOAD_DIR="/tmp/webserv_uploads"
NO_PERMS_DIR="/tmp/webserv_no_perms"

GET_FILE="$GET_ROOT/hello.txt"
FORBIDDEN_FILE="$NO_PERMS_DIR/secret.txt"
UPLOAD_SRC_FILE="/tmp/upload_this.txt"
UPLOAD_DST_FILE="$UPLOAD_DIR/uploaded_file.bin"

PORT=8081
ADDRESS="127.0.0.1:$PORT"

# --- Helper Functions ---

function setup_test_env() {
    echo -e "${BLUE}Setting up test environment...${NC}"
    mkdir -p "$GET_ROOT"
    mkdir -p "$NO_AUTOINDEX_DIR"
    mkdir -p "$UPLOAD_DIR"
    mkdir -p "$NO_PERMS_DIR"

    echo "Hello from webserv test!" > "$GET_FILE"
    echo "This is a file to be uploaded." > "$UPLOAD_SRC_FILE"
    echo "secret content" > "$FORBIDDEN_FILE"
    chmod 000 "$FORBIDDEN_FILE"

    echo "Building webserv..."
    make > /dev/null
    if [ ! -f "$WEBSERV_EXEC" ]; then
        echo -e "${RED}FAIL: webserv executable not found after make${NC}"
        exit 1
    fi
}

function cleanup() {
    echo -e "\n${BLUE}Cleaning up...${NC}"
    # Restore permissions so rm can delete it
    chmod 755 "$FORBIDDEN_FILE" || true

    if [ ! -z "$WEBSERV_PID" ]; then
        # Check if process exists before killing
        if kill -0 $WEBSERV_PID 2>/dev/null; then
            kill $WEBSERV_PID
            wait $WEBSERV_PID 2>/dev/null
        fi
    fi
    rm -rf "$GET_ROOT"
    rm -rf "$UPLOAD_DIR"
    rm -rf "$NO_PERMS_DIR"
    rm -f "$UPLOAD_SRC_FILE"
}

trap cleanup EXIT

# --- Main Test Logic ---

setup_test_env

echo "Starting webserv for methods test..."
./webserv "$TEST_CONF" & 
WEBSERV_PID=$!
sleep 1

# Check if process is running
if ! kill -0 $WEBSERV_PID 2>/dev/null; then
    echo -e "${RED}FAIL: webserv process did not start or crashed immediately${NC}"
    exit 1
fi
echo "Webserv started with PID: $WEBSERV_PID on port $PORT"

# ===================
# === GET Tests ===
# ===================
echo -e "\n${BLUE}--- Testing GET... ---${NC}"

# Test 1: GET 200 OK
HTTP_STATUS=$(curl -s -o /dev/null -w '%{http_code}' "http://$ADDRESS/hello.txt")
RESPONSE_BODY=$(curl -s "http://$ADDRESS/hello.txt")
EXPECTED_BODY=$(cat "$GET_FILE")
if [[ "$HTTP_STATUS" -ne 200 || "$RESPONSE_BODY" != "$EXPECTED_BODY" ]]; then
    echo -e "${RED}[GET 200] FAIL: Expected 200 and correct body, got $HTTP_STATUS${NC}"
    exit 1
fi
echo -e "${GREEN}[GET 200] OK${NC}"

# Test 2: GET 404 Not Found
HTTP_STATUS=$(curl -s -o /dev/null -w '%{http_code}' "http://$ADDRESS/non_existent_file.txt")
if [[ "$HTTP_STATUS" -ne 404 ]]; then
    echo -e "${RED}[GET 404] FAIL: Expected 404, got $HTTP_STATUS${NC}"
    exit 1
fi
echo -e "${GREEN}[GET 404] OK${NC}"

# Test 3: GET 403 Forbidden (File Permissions)
HTTP_STATUS=$(curl -s -o /dev/null -w '%{http_code}' "http://$ADDRESS/forbidden/secret.txt")
if [[ "$HTTP_STATUS" -ne 403 ]]; then
    echo -e "${RED}[GET 403] FAIL: Expected 403, got $HTTP_STATUS${NC}"
    exit 1
fi
echo -e "${GREEN}[GET 403] OK${NC}"

# Test 4: GET Directory with autoindex on
HTTP_STATUS=$(curl -s -o /dev/null -w '%{http_code}' "http://$ADDRESS/autoindex_on/")
RESPONSE_BODY=$(curl -s "http://$ADDRESS/autoindex_on/")
if [[ "$HTTP_STATUS" -ne 200 || "$RESPONSE_BODY" != *"hello.txt"* ]]; then
    echo -e "${RED}[GET autoindex] FAIL: Expected 200 and directory listing, got $HTTP_STATUS${NC}"
    exit 1
fi
echo -e "${GREEN}[GET autoindex] OK${NC}"

# Test 5: GET Directory with autoindex off
HTTP_STATUS=$(curl -s -o /dev/null -w '%{http_code}' "http://$ADDRESS/no_autoindex/")
if [[ "$HTTP_STATUS" -ne 403 ]]; then
    echo -e "${RED}[GET no autoindex] FAIL: Expected 403, got $HTTP_STATUS${NC}"
    exit 1
fi
echo -e "${GREEN}[GET no autoindex] OK${NC}"


# ====================
# === POST Tests ===
# ====================
echo -e "\n${BLUE}--- Testing POST... ---${NC}"

# Test 1: POST 201 Created (Upload)
HTTP_STATUS=$(curl -s -o /dev/null -w '%{http_code}' -X POST --data-binary "@$UPLOAD_SRC_FILE" "http://$ADDRESS/upload/ignored_filename")
if [[ "$HTTP_STATUS" -ne 201 ]]; then
    echo -e "${RED}[POST 201] FAIL: Expected status 201, got $HTTP_STATUS${NC}"
    exit 1
fi
if [[ ! -f "$UPLOAD_DST_FILE" ]]; then
    echo -e "${RED}[POST 201] FAIL: Uploaded file was not created at $UPLOAD_DST_FILE${NC}"
    exit 1
fi
if ! diff -q "$UPLOAD_SRC_FILE" "$UPLOAD_DST_FILE" >/dev/null 2>&1; then
    echo -e "${RED}[POST 201] FAIL: Content of uploaded file does not match source file${NC}"
    exit 1
fi
echo -e "${GREEN}[POST 201] OK${NC}"

# Test 2: POST 405 Method Not Allowed
HTTP_STATUS=$(curl -s -o /dev/null -w '%{http_code}' -X POST --data "test" "http://$ADDRESS/hello.txt")
if [[ "$HTTP_STATUS" -ne 405 ]]; then
    echo -e "${RED}[POST 405] FAIL: Expected 405, got $HTTP_STATUS${NC}"
    exit 1
fi
echo -e "${GREEN}[POST 405] OK${NC}"


# =====================
# === DELETE Tests ===
# =====================
echo -e "\n${BLUE}--- Testing DELETE... ---${NC}"

# Test 1: DELETE 204 No Content
HTTP_STATUS=$(curl -s -o /dev/null -w '%{http_code}' -X DELETE "http://$ADDRESS/upload/uploaded_file.bin")
if [[ "$HTTP_STATUS" -ne 204 || -f "$UPLOAD_DST_FILE" ]]; then
    echo -e "${RED}[DELETE 204] FAIL: Expected 204 and file deletion, got $HTTP_STATUS${NC}"
    exit 1
fi
echo -e "${GREEN}[DELETE 204] OK${NC}"

# Test 2: DELETE 404 Not Found
HTTP_STATUS=$(curl -s -o /dev/null -w '%{http_code}' -X DELETE "http://$ADDRESS/upload/non_existent_file")
if [[ "$HTTP_STATUS" -ne 404 ]]; then
    echo -e "${RED}[DELETE 404] FAIL: Expected 404, got $HTTP_STATUS${NC}"
    exit 1
fi
echo -e "${GREEN}[DELETE 404] OK${NC}"

# Test 3: DELETE 405 Method Not Allowed
HTTP_STATUS=$(curl -s -o /dev/null -w '%{http_code}' -X DELETE "http://$ADDRESS/hello.txt")
if [[ "$HTTP_STATUS" -ne 405 ]]; then
    echo -e "${RED}[DELETE 405] FAIL: Expected 405, got $HTTP_STATUS${NC}"
    exit 1
fi
echo -e "${GREEN}[DELETE 405] OK${NC}"


echo -e "\n${GREEN}SUCCESS: All HTTP method tests passed!${NC}"
exit 0
