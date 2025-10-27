#!/bin/bash

WEBSERV_BIN="./webserv"
TEST_ROOT="./www/http_test_root"
FORBIDDEN_DIR="./www/forbidden_dir"

# Colors
GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m' # No Color

# Setup function
setup_test_env() {
    echo "Setting up test environment..."
    mkdir -p "$TEST_ROOT"
    mkdir -p "$TEST_ROOT/no_autoindex_dir"
    mkdir -p "$TEST_ROOT/autoindex_test_dir"
    mkdir -p "$FORBIDDEN_DIR"

    # Create test files
    echo "Hello from webserv test!" > "$TEST_ROOT/hello.txt"
    echo "<html><body><h1>Welcome!</h1></body></html>" > "$TEST_ROOT/index.html"

    # Create forbidden directory with no permissions
    chmod 000 "$FORBIDDEN_DIR"
}

# Cleanup function
cleanup() {
    echo "Cleaning up test environment..."
    # Restore permissions before deletion
    chmod 755 "$FORBIDDEN_DIR" 2>/dev/null || true
    rm -rf "$TEST_ROOT" "$FORBIDDEN_DIR" 2>/dev/null || true
}

trap cleanup EXIT

# Function to run a test case
run_test() {
    local config_file=$1
    local test_name=$2
    local path=$3
    local expected_status=$4
    local expected_content_grep=$5 # Use grep pattern for content

    echo "Running test: $test_name (Config: $config_file, Path: $path)"

    # Start the webserv in the background
    $WEBSERV_BIN "$config_file" &
    WEBSERV_PID=$!

    # Give the server a moment to start up
    sleep 1

    # Get HTTP status code
    HTTP_STATUS=$(curl -s -o /dev/null -w '%{http_code}' "http://localhost:8080$path")

    # Get response headers
    RESPONSE_HEADERS=$(curl -s -I "http://localhost:8080$path" 2>&1 | grep -i -E "^HTTP|^Content-Type|^Content-Length")

    # Get response body
    RESPONSE_BODY=$(curl -s "http://localhost:8080$path")

    # Check for expected status code
    if [[ "$HTTP_STATUS" == "$(echo "$expected_status" | cut -d' ' -f1)" ]]; then
        echo -e "  ${GREEN}Success: Status code $expected_status found.${NC}"
    else
        echo -e "  ${RED}Error: Status code $expected_status not found. Actual: $HTTP_STATUS.${NC} Headers: $RESPONSE_HEADERS"
        kill $WEBSERV_PID
        return 1
    fi

    # Check for expected content (if applicable)
    if [ -n "$expected_content_grep" ]; then
        if echo "$RESPONSE_BODY" | grep -q "$expected_content_grep"; then
            echo -e "  ${GREEN}Success: Expected content found.${NC}"
        else
            echo -e "  ${RED}Error: Expected content '$expected_content_grep' not found in body.${NC} Body: $RESPONSE_BODY"
            kill $WEBSERV_PID
            return 1
        fi
    fi

    # Kill the webserv process
    kill $WEBSERV_PID
    echo ""
    return 0
}

# Setup test environment
setup_test_env

# --- Test Cases ---

# 1. Serving a basic static file
run_test "test/confs/valid/config_basic_get.yaml" "Basic Static File" "/hello.txt" "200 OK" "Hello from webserv test!" || exit 1

# 2. Directory listing (autoindex on)
run_test "test/confs/valid/config_autoindex_on.yaml" "Autoindex On" "/no_autoindex_dir/" "200 OK" "Index of /no_autoindex_dir/" || exit 1

# 3. Default file (indexFile)
run_test "test/confs/valid/config_index_file.yaml" "Index File" "/" "200 OK" "Welcome!" || exit 1

# 4. Directory listing (autoindex off) - should be 403 Forbidden
run_test "test/confs/valid/config_autoindex_off.yaml" "Autoindex Off (Forbidden)" "/no_autoindex_dir/" "403 Forbidden" "Forbidden" || exit 1

# 5. File not found
run_test "test/confs/valid/config_basic_get.yaml" "File Not Found" "/non-existent-file.txt" "404 Not Found" "Not Found" || exit 1

# 6. Forbidden access to directory
run_test "test/confs/valid/config_forbidden.yaml" "Forbidden Directory Access" "/forbidden_dir/" "500 Internal Server Error" "Internal Server Error" || exit 1


echo -e "${GREEN}All static file tests passed!${NC}"
exit 0
