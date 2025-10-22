#!/bin/bash

WEBSERV_BIN="./webserv"
TEST_DIR="test/validation_test"

GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m' # No Color

error_count=0

# Function to run a test case
run_test() {
    local config_file=$1
    local description=$2
    local expected_error_msg=$3

    echo -n "Testing $description... "

    # Run the server and capture stderr
    error_output=$($WEBSERV_BIN "$config_file" 2>&1)
    exit_code=$?

    # Check for non-zero exit code
    if [ $exit_code -eq 0 ]; then
        echo -e "${RED}FAIL${NC}"
        echo "  - Expected a non-zero exit code, but got 0."
        error_count=$((error_count + 1))
        return
    fi

    # Check for the specific error message
    if ! echo "$error_output" | grep -q "$expected_error_msg"; then
        echo -e "${RED}FAIL${NC}"
        echo "  - Expected error message containing '$expected_error_msg'"
        echo "  - Got: $error_output"
        error_count=$((error_count + 1))
        return
    fi

    echo -e "${GREEN}PASS${NC}"
}

# Build the executable first
make
if [ $? -ne 0 ]; then
    echo -e "${RED}Build failed. Aborting tests.${NC}"
    exit 1
fi

# --- Run Tests for Invalid Configs ---
run_test "$TEST_DIR/test_invalid_key_server.yaml" "unknown key in server block" "unknown directive 'maxEvets'"
run_test "$TEST_DIR/test_invalid_key_location.yaml" "unknown key in location block" "unknown directive 'invalid_key'"
run_test "$TEST_DIR/test_invalid_key_listen.yaml" "unknown key in listen block" "unknown directive 'invalid_key'"
run_test "$TEST_DIR/test_invalid_key_log.yaml" "unknown key in error_log block" "unknown directive 'invalid_key'"

# --- Test a valid config to ensure it doesn't fail ---
echo -n "Testing valid config (config/default.yaml)... "
$WEBSERV_BIN config/default.yaml & 
WEBSERV_PID=$!
sleep 2 # Give it a moment to potentially crash

# Check if the process is still running
if ps -p $WEBSERV_PID > /dev/null; then
    echo -e "${GREEN}PASS${NC}"
    kill $WEBSERV_PID
else
    echo -e "${RED}FAIL${NC}"
    echo "  - Server with valid config failed to start."
    error_count=$((error_count + 1))
fi


# --- Final Result ---
if [ $error_count -eq 0 ]; then
    echo -e "\n${GREEN}All validation tests passed!${NC}"
    exit 0
else
    echo -e "\n${RED}$error_count test(s) failed.${NC}"
    exit 1
fi
