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
TEST_CONFIG="$PROJECT_ROOT/test/cgi_test/config.yaml"

PORT=8082
ADDRESS="127.0.0.1:$PORT"

overall_status=0

# --- Helper Functions ---

function cleanup() {
    echo -e "${BLUE}Cleaning up...${NC}"
    if [ -n "$WEBSERV_PID" ]; then
        kill $WEBSERV_PID 2>/dev/null || true
        wait $WEBSERV_PID 2>/dev/null || true
    fi
}

trap cleanup EXIT

# Function to run a test case
run_test() {
    local test_name=$1
    local path=$2
    local method=$3
    local data=$4
    local expected_status=$5
    local expected_content=$6

    local url="http://${ADDRESS}${path}"

    echo "Running test: $test_name"

    # Build curl command
    local curl_cmd="curl -s"
    if [ "$method" = "POST" ] && [ -n "$data" ]; then
        curl_cmd="$curl_cmd -X POST -d '$data'"
    fi

    # Get HTTP status
    HTTP_STATUS=$(eval "$curl_cmd -o /dev/null -w '%{http_code}' '$url'")
    
    # Get response body
    RESPONSE_BODY=$(eval "$curl_cmd '$url'")

    # Check status code
    if [[ "$HTTP_STATUS" == "$expected_status" ]]; then
        echo -e "  ${GREEN}Success: Status code $expected_status found.${NC}"
    else
        echo -e "  ${RED}Error: Expected status $expected_status, got $HTTP_STATUS.${NC}"
        overall_status=1
        echo ""
        return 1
    fi

    # Check content if provided
    if [ -n "$expected_content" ]; then
        if echo "$RESPONSE_BODY" | grep -q "$expected_content"; then
            echo -e "  ${GREEN}Success: Expected content found.${NC}"
        else
            echo -e "  ${RED}Error: Expected content '$expected_content' not found.${NC}"
            echo "  Response: $RESPONSE_BODY"
            overall_status=1
            echo ""
            return 1
        fi
    fi

    echo ""
    return 0
}

# --- Main Test Execution ---

echo -e "${BLUE}Building webserv...${NC}"
make -C "$PROJECT_ROOT" > /dev/null

echo -e "${BLUE}Starting webserv for CGI tests...${NC}"
cd "$PROJECT_ROOT"
$WEBSERV_EXEC "$TEST_CONFIG" > /dev/null 2>&1 &
WEBSERV_PID=$!
sleep 2

# Check if server started
if ! kill -0 $WEBSERV_PID 2>/dev/null; then
    echo -e "${RED}FAIL: webserv process did not start or crashed immediately${NC}"
    exit 1
fi
echo -e "Webserv started with PID: ${WEBSERV_PID} on port ${PORT}\n"

# --- Test Cases ---

echo -e "${BLUE}--- Testing CGI... ---${NC}"

# Test 1: Simple CGI GET request
run_test "Simple CGI GET" \
         "/cgi-bin/simple.py" \
         "GET" \
         "" \
         "200" \
         "Hello from CGI!"

# Test 2: Echo CGI with GET (no data)
run_test "Echo CGI GET" \
         "/cgi-bin/echo.py" \
         "GET" \
         "" \
         "200" \
         "Echo CGI Script"

# Test 3: Echo CGI with POST data
run_test "Echo CGI POST" \
         "/cgi-bin/echo.py" \
         "POST" \
         "test_data=hello&name=world" \
         "200" \
         "test_data=hello"

# Test 4: Echo CGI with query string
run_test "Echo CGI with Query String" \
         "/cgi-bin/echo.py?foo=bar&baz=qux" \
         "GET" \
         "" \
         "200" \
         "foo=bar"

# Test 5: Non-existent CGI script (should return 404)
run_test "Non-existent CGI" \
         "/cgi-bin/nonexistent.py" \
         "GET" \
         "" \
         "404" \
         ""

# --- Test Result ---

if [ $overall_status -eq 0 ]; then
    echo -e "\n${GREEN}✅ All CGI tests passed!${NC}"
    exit 0
else
    echo -e "\n${RED}❌ Some CGI tests failed!${NC}"
    exit 1
fi

