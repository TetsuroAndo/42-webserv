#!/bin/bash

# Session Test Script
# Tests cookie and session management functionality

TEST_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "$TEST_DIR/../.." && pwd)"
CONFIG="$TEST_DIR/config.yaml"
PORT=8082
PID_FILE="/tmp/webserv_session_test.pid"
SERVER_BIN="$ROOT_DIR/webserv"

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Test counter
TESTS_PASSED=0
TESTS_FAILED=0

# Cleanup function
cleanup() {
    if [ -f "$PID_FILE" ]; then
        PID=$(cat "$PID_FILE")
        if kill -0 "$PID" 2>/dev/null; then
            kill "$PID" 2>/dev/null
            sleep 1
            kill -9 "$PID" 2>/dev/null
        fi
        rm -f "$PID_FILE"
    fi
    cleanup_test_files
}

# Error handler
error_exit() {
    echo -e "${RED}Error: $1${NC}" >&2
    cleanup
    exit 1
}

# Setup test files
setup_test_files() {
    # Create index.html if it doesn't exist
    if [ ! -f "$ROOT_DIR/www/index.html" ]; then
        echo "<html><body>Session Test Page</body></html>" > "$ROOT_DIR/www/index.html"
        echo "Created temporary index.html"
        echo "TEMP_INDEX_CREATED" > /tmp/session_test_created_index
    fi
}

# Cleanup test files
cleanup_test_files() {
    # Remove index.html if we created it
    if [ -f /tmp/session_test_created_index ]; then
        rm -f "$ROOT_DIR/www/index.html"
        rm -f /tmp/session_test_created_index
        echo "Removed temporary index.html"
    fi
}

# Start server
start_server() {
    echo "Starting webserv on port $PORT..."
    "$SERVER_BIN" "$CONFIG" > /dev/null 2>&1 &
    SERVER_PID=$!
    echo $SERVER_PID > "$PID_FILE"
    sleep 2
    
    if ! kill -0 $SERVER_PID 2>/dev/null; then
        error_exit "Failed to start webserv"
    fi
    echo "Webserv started with PID: $SERVER_PID"
}

# Test function
run_test() {
    local test_name="$1"
    local expected="$2"
    shift 2
    
    echo ""
    echo "Running test: $test_name"
    
    # Run the test command
    result=$("$@")
    
    if echo "$result" | grep -q "$expected"; then
        echo -e "  ${GREEN}Success${NC}: $test_name"
        TESTS_PASSED=$((TESTS_PASSED + 1))
        return 0
    else
        echo -e "  ${RED}Error${NC}: $test_name"
        echo "  Expected: $expected"
        echo "  Got: $result"
        TESTS_FAILED=$((TESTS_FAILED + 1))
        return 1
    fi
}

# Setup
trap cleanup EXIT
cleanup

# Build if needed
if [ ! -f "$SERVER_BIN" ]; then
    echo "Building webserv..."
    cd "$ROOT_DIR" && make -j4 > /dev/null 2>&1 || error_exit "Build failed"
fi

# Setup test files
setup_test_files

start_server

# Wait for server to be ready
sleep 1

echo ""
echo "=========================================="
echo "Testing Session Management"
echo "=========================================="

# Test 1: First request should create a session and return Set-Cookie header
echo ""
echo "Test 1: Session Creation"
RESPONSE=$(curl -i -s http://127.0.0.1:$PORT/)
if echo "$RESPONSE" | grep -q "Set-Cookie.*sessionId="; then
    echo -e "  ${GREEN}Success${NC}: Session cookie is set"
    SESSION_ID=$(echo "$RESPONSE" | grep "Set-Cookie.*sessionId=" | sed 's/.*sessionId="\([^"]*\)".*/\1/' | head -1)
    echo "  Session ID: $SESSION_ID"
    TESTS_PASSED=$((TESTS_PASSED + 1))
else
    echo -e "  ${RED}Error${NC}: Session cookie not found"
    echo "$RESPONSE"
    TESTS_FAILED=$((TESTS_FAILED + 1))
fi

# Test 2: Subsequent request with session cookie should maintain session
echo ""
echo "Test 2: Session Persistence"
if [ -n "$SESSION_ID" ]; then
    RESPONSE=$(curl -i -s -b "sessionId=\"$SESSION_ID\"" http://127.0.0.1:$PORT/)
    if echo "$RESPONSE" | grep -q "Set-Cookie.*sessionId=\"$SESSION_ID\""; then
        echo -e "  ${GREEN}Success${NC}: Session ID persisted"
        TESTS_PASSED=$((TESTS_PASSED + 1))
    else
        echo -e "  ${RED}Error${NC}: Session ID not persisted"
        echo "$RESPONSE"
        TESTS_FAILED=$((TESTS_FAILED + 1))
    fi
else
    echo -e "  ${YELLOW}Skipped${NC}: No session ID from previous test"
fi

# Test 3: Multiple Set-Cookie headers (sessionId, lastAccessTime, serverName)
echo ""
echo "Test 3: Multiple Cookie Headers"
RESPONSE=$(curl -i -s http://127.0.0.1:$PORT/)
COOKIE_COUNT=$(echo "$RESPONSE" | grep -c "Set-Cookie:")
if [ "$COOKIE_COUNT" -ge 3 ]; then
    echo -e "  ${GREEN}Success${NC}: Multiple cookies set (Count: $COOKIE_COUNT)"
    TESTS_PASSED=$((TESTS_PASSED + 1))
else
    echo -e "  ${RED}Error${NC}: Expected at least 3 Set-Cookie headers, got $COOKIE_COUNT"
    TESTS_FAILED=$((TESTS_FAILED + 1))
fi

# Test 4: Check for HttpOnly flag on sessionId cookie
echo ""
echo "Test 4: HttpOnly Flag"
RESPONSE=$(curl -i -s http://127.0.0.1:$PORT/)
if echo "$RESPONSE" | grep "Set-Cookie.*sessionId=" | grep -q "HttpOnly"; then
    echo -e "  ${GREEN}Success${NC}: HttpOnly flag is set on sessionId"
    TESTS_PASSED=$((TESTS_PASSED + 1))
else
    echo -e "  ${RED}Error${NC}: HttpOnly flag not found on sessionId"
    TESTS_FAILED=$((TESTS_FAILED + 1))
fi

# Test 5: Check for Path=/ on all cookies
echo ""
echo "Test 5: Cookie Path"
RESPONSE=$(curl -i -s http://127.0.0.1:$PORT/)
COOKIES_WITH_PATH=$(echo "$RESPONSE" | grep "Set-Cookie:" | grep -c "Path=/")
if [ "$COOKIES_WITH_PATH" -ge 3 ]; then
    echo -e "  ${GREEN}Success${NC}: All cookies have Path=/ set"
    TESTS_PASSED=$((TESTS_PASSED + 1))
else
    echo -e "  ${RED}Error${NC}: Not all cookies have Path=/ set"
    TESTS_FAILED=$((TESTS_FAILED + 1))
fi

# Test 6: Verify serverName cookie contains correct value
echo ""
echo "Test 6: Server Name Cookie"
RESPONSE=$(curl -i -s http://127.0.0.1:$PORT/)
if echo "$RESPONSE" | grep "Set-Cookie.*serverName=webserv/42"; then
    echo -e "  ${GREEN}Success${NC}: serverName cookie has correct value"
    TESTS_PASSED=$((TESTS_PASSED + 1))
else
    echo -e "  ${RED}Error${NC}: serverName cookie value incorrect"
    TESTS_FAILED=$((TESTS_FAILED + 1))
fi

# Test 7: Verify lastAccessTime cookie is set
echo ""
echo "Test 7: Last Access Time Cookie"
RESPONSE=$(curl -i -s http://127.0.0.1:$PORT/)
if echo "$RESPONSE" | grep -q "Set-Cookie.*lastAccessTime="; then
    echo -e "  ${GREEN}Success${NC}: lastAccessTime cookie is set"
    TESTS_PASSED=$((TESTS_PASSED + 1))
else
    echo -e "  ${RED}Error${NC}: lastAccessTime cookie not found"
    TESTS_FAILED=$((TESTS_FAILED + 1))
fi

# Test 8: Session ID uniqueness - multiple requests should get different session IDs
echo ""
echo "Test 8: Session ID Uniqueness"
SESSION_ID_1=$(curl -i -s http://127.0.0.1:$PORT/ | grep "Set-Cookie.*sessionId=" | sed 's/.*sessionId="\([^"]*\)".*/\1/' | head -1)
SESSION_ID_2=$(curl -i -s http://127.0.0.1:$PORT/ | grep "Set-Cookie.*sessionId=" | sed 's/.*sessionId="\([^"]*\)".*/\1/' | head -1)
if [ "$SESSION_ID_1" != "$SESSION_ID_2" ] && [ -n "$SESSION_ID_1" ] && [ -n "$SESSION_ID_2" ]; then
    echo -e "  ${GREEN}Success${NC}: Session IDs are unique"
    echo "  Session 1: $SESSION_ID_1"
    echo "  Session 2: $SESSION_ID_2"
    TESTS_PASSED=$((TESTS_PASSED + 1))
else
    echo -e "  ${RED}Error${NC}: Session IDs are not unique or empty"
    echo "  Session 1: $SESSION_ID_1"
    echo "  Session 2: $SESSION_ID_2"
    TESTS_FAILED=$((TESTS_FAILED + 1))
fi

# Test 9: Cookie parsing - send cookies with quotes
echo ""
echo "Test 9: Cookie Parsing with Quotes"
RESPONSE=$(curl -i -s -b "sessionId=\"test123\"" http://127.0.0.1:$PORT/)
if echo "$RESPONSE" | grep -q "HTTP/1.1 200"; then
    echo -e "  ${GREEN}Success${NC}: Server handles quoted cookie values"
    TESTS_PASSED=$((TESTS_PASSED + 1))
else
    echo -e "  ${RED}Error${NC}: Server failed to handle quoted cookie values"
    TESTS_FAILED=$((TESTS_FAILED + 1))
fi

# Test 10: Multiple cookies in request
echo ""
echo "Test 10: Multiple Cookies in Request"
RESPONSE=$(curl -i -s -b "sessionId=\"test123\"; lastAccessTime=2023-01-01; serverName=test" http://127.0.0.1:$PORT/)
if echo "$RESPONSE" | grep -q "HTTP/1.1 200"; then
    echo -e "  ${GREEN}Success${NC}: Server handles multiple cookies"
    TESTS_PASSED=$((TESTS_PASSED + 1))
else
    echo -e "  ${RED}Error${NC}: Server failed to handle multiple cookies"
    TESTS_FAILED=$((TESTS_FAILED + 1))
fi

echo ""
echo "=========================================="
echo "Test Summary"
echo "=========================================="
echo "Total tests: $((TESTS_PASSED + TESTS_FAILED))"
echo -e "Passed: ${GREEN}$TESTS_PASSED${NC}"
echo -e "Failed: ${RED}$TESTS_FAILED${NC}"
echo "=========================================="

cleanup

if [ $TESTS_FAILED -eq 0 ]; then
    echo -e "${GREEN}All session tests passed!${NC}"
    exit 0
else
    echo -e "${RED}Some session tests failed!${NC}"
    exit 1
fi
