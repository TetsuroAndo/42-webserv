#!/bin/bash

WEBSERV_BIN="./webserv"
UPLOAD_DIR="./tmp/default_uploads"
API_UPLOAD_DIR="./www/upload"
TEST_ROOT="./www/http_test_root"

# Setup function
setup_test_files() {
    mkdir -p "$UPLOAD_DIR"
    mkdir -p "$API_UPLOAD_DIR"
    mkdir -p "$TEST_ROOT"
    echo "<html><body>Test</body></html>" > "$TEST_ROOT/index.html"
    echo "Hello" > "$TEST_ROOT/hello.txt"
}

# Cleanup function
cleanup() {
    rm -rf "$UPLOAD_DIR" "$API_UPLOAD_DIR" "$TEST_ROOT" 2>/dev/null || true
}

trap cleanup EXIT

# Setup
setup_test_files

# Colors
GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m' # No Color

# Function to run a generic test case
run_curl_test() {
    local test_name=$1
    local config_file=$2
    local curl_args=$3
    local expected_status=$4

    echo "Running test: $test_name"

    # Start the webserv in the background
    $WEBSERV_BIN "$config_file" &
    WEBSERV_PID=$!
    sleep 1

    # Run curl and get status
    HTTP_STATUS=$(eval curl -s -o /dev/null -w '%{http_code}' $curl_args)

    # Check status
    if [[ "$HTTP_STATUS" == "$expected_status" ]]; then
        echo -e "  ${GREEN}Success: Status code $expected_status found.${NC}"
    else
        echo -e "  ${RED}Error: Status code $expected_status not found. Actual: $HTTP_STATUS.${NC}"
        kill $WEBSERV_PID
        exit 1
    fi

    kill $WEBSERV_PID
    echo ""
}


# --- Test Case 1: Invalid Method in Config ---
echo "Running test: Invalid Method (PUT)"
ERROR_OUTPUT=$($WEBSERV_BIN test/allowed_method_test/config_invalid_method.yaml 2>&1)
if echo "$ERROR_OUTPUT" | grep -q "Config error: invalid HTTP method 'PUT'"; then
    echo -e "  ${GREEN}Success: Server failed to start with the expected error message.${NC}"
else
    echo -e "  ${RED}Error: Server did not fail as expected or the error message was wrong.${NC}"
    echo "    Output: $ERROR_OUTPUT"
    exit 1
fi
echo ""


# --- Test Case 2: No allowedMethods in Config ---
run_curl_test "No allowedMethods (GET)" \
              "test/allowed_method_test/config_no_methods.yaml" \
              "http://localhost:8080/" \
              "200"

run_curl_test "No allowedMethods (POST)" \
              "test/allowed_method_test/config_no_methods.yaml" \
              "-X POST -H 'Content-Type: text/plain' --data 'test' http://localhost:8080/" \
              "201"


# --- Test Case 3: Disallowed Method ---
run_curl_test "Disallowed Method (POST to GET-only)" \
              "test/allowed_method_test/config_get_only.yaml" \
              "-X POST http://localhost:8080/" \
              "405"


# --- Test Case 4: Multiple Locations ---
run_curl_test "Multiple Locations (GET on /)" \
              "test/allowed_method_test/config_multiple_locations.yaml" \
              "http://localhost:8080/" \
              "200"

run_curl_test "Multiple Locations (POST on /)" \
              "test/allowed_method_test/config_multiple_locations.yaml" \
              "-X POST http://localhost:8080/" \
              "405"

run_curl_test "Multiple Locations (POST on /api)" \
              "test/allowed_method_test/config_multiple_locations.yaml" \
              "-X POST --data 'test' http://localhost:8080/api" \
              "201"

run_curl_test "Multiple Locations (GET on /api)" \
              "test/allowed_method_test/config_multiple_locations.yaml" \
              "http://localhost:8080/api" \
              "405"


echo -e "${GREEN}All method tests passed!${NC}"
exit 0