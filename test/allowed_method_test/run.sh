#!/bin/bash

# Get the project root directory (2 levels up from this script)
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"
WEBSERV_BIN="$PROJECT_ROOT/webserv"

# Colors
GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m' # No Color

# Create necessary directories
mkdir -p "$PROJECT_ROOT/www/upload"

# Function to run a generic test case
run_curl_test() {
    local test_name=$1
    local config_file=$2
    local curl_args=$3
    local expected_status=$4

    echo "Running test: $test_name"

    # Start the webserv in the background from project root
    cd "$PROJECT_ROOT"
    $WEBSERV_BIN "$config_file" &
    WEBSERV_PID=$!
    sleep 1

    # Run curl and get status
    HTTP_STATUS=$(curl -s -o /dev/null -w '%{http_code}' $curl_args)

    # Check status
    if [[ "$HTTP_STATUS" == "$expected_status" ]]; then
        echo -e "  ${GREEN}Success: Status code $expected_status found.${NC}"
    else
        echo -e "  ${RED}Error: Status code $expected_status not found. Actual: $HTTP_STATUS.${NC}"
        kill $WEBSERV_PID
        exit 1
    fi

    kill $WEBSERV_PID
    sleep 0.5
    echo ""
}


# --- Test Case 1: Invalid Method in Config ---
echo "Running test: Invalid Method (PUT)"
ERROR_OUTPUT=$($WEBSERV_BIN "$PROJECT_ROOT/test/allowed_method_test/config_invalid_method.yaml" 2>&1)
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
              "$PROJECT_ROOT/test/allowed_method_test/config_no_methods.yaml" \
              "http://localhost:8080/" \
              "405"

run_curl_test "No allowedMethods (POST)" \
              "$PROJECT_ROOT/test/allowed_method_test/config_no_methods.yaml" \
              "-X POST http://localhost:8080/" \
              "405"


# --- Test Case 3: Disallowed Method ---
run_curl_test "Disallowed Method (POST to GET-only)" \
              "$PROJECT_ROOT/test/allowed_method_test/config_get_only.yaml" \
              "-X POST http://localhost:8080/" \
              "405"


# --- Test Case 4: Multiple Locations ---
run_curl_test "Multiple Locations (GET on /)" \
              "$PROJECT_ROOT/test/allowed_method_test/config_multiple_locations.yaml" \
              "http://localhost:8080/" \
              "200"

run_curl_test "Multiple Locations (POST on /)" \
              "$PROJECT_ROOT/test/allowed_method_test/config_multiple_locations.yaml" \
              "-X POST http://localhost:8080/" \
              "405"

run_curl_test "Multiple Locations (POST on /api)" \
              "$PROJECT_ROOT/test/allowed_method_test/config_multiple_locations.yaml" \
              "-X POST --data 'test' http://localhost:8080/api" \
              "201"

run_curl_test "Multiple Locations (GET on /api)" \
              "$PROJECT_ROOT/test/allowed_method_test/config_multiple_locations.yaml" \
              "http://localhost:8080/api" \
              "405"


echo -e "${GREEN}All method tests passed!${NC}"
exit 0