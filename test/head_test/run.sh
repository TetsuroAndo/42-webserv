#!/bin/bash

# Get the project root directory (2 levels up from this script)
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"
WEBSERV_BIN="$PROJECT_ROOT/webserv"

# Colors
GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m' # No Color

overall_status=0 # 0 for success, 1 for failure

# Function to run a test case
run_test() {
    local config_file=$1
    local test_name=$2
    local path=$3
    local expected_status=$4
    local expected_location=$5
    local expected_content_length=$6
    local curl_opts=$7

    echo "Running test: $test_name (Config: $config_file, Path: $path)"

    # Start the webserv in the background from project root
    cd "$PROJECT_ROOT"
    $WEBSERV_BIN "$config_file" &
    WEBSERV_PID=$!

    # Give the server a moment to start up
    sleep 1

    # Get response for HEAD request
    # Use -I instead of -X HEAD for proper HEAD request handling
    HEAD_RESPONSE=$(curl -s $curl_opts -I "http://localhost:8081$path" 2>&1)
    # Get HTTP status code for HEAD request
    HEAD_HTTP_STATUS=$(echo "$HEAD_RESPONSE" | grep -i -E "^HTTP" | awk '{print $2}')
    # Get headers
    HEAD_RESPONSE_HEADERS=$(echo "$HEAD_RESPONSE" | sed -n '1,/^\r$/p')
    # Get response body for HEAD request
    HEAD_RESPONSE_BODY=$(echo "$HEAD_RESPONSE" | sed '1,/^\r$/d')

    # Check for expected status code
    if [[ "$HEAD_HTTP_STATUS" == "$expected_status" ]]; then
        echo -e "  ${GREEN}Success: Status code $expected_status found.${NC}"
    else
        echo -e "  ${RED}Error: Status code $expected_status not found. Actual: $HEAD_HTTP_STATUS.${NC} Headers: $HEAD_RESPONSE_HEADERS"
        kill $WEBSERV_PID
        return 1
    fi

    # For 200 OK, perform additional checks
    if [[ "$expected_status" == "200" ]]; then
        # Get Content-Length for HEAD request
        HEAD_CONTENT_LENGTH=$(echo "$HEAD_RESPONSE_HEADERS" | grep -i -E "^Content-Length" | awk '{print $2}' | tr -d '\r')

        # Check that HEAD response body is empty
        if [[ -z "$HEAD_RESPONSE_BODY" ]]; then
            echo -e "  ${GREEN}Success: HEAD response body is empty.${NC}"
        else
            echo -e "  ${RED}Error: HEAD response body is not empty. Body: $HEAD_RESPONSE_BODY${NC}"
            kill $WEBSERV_PID
            return 1
        fi
        
        if [[ -n "$expected_content_length" ]]; then
            if [[ "$HEAD_CONTENT_LENGTH" == "$expected_content_length" ]]; then
                echo -e "  ${GREEN}Success: Content-Length is correct.${NC}"
            else
                echo -e "  ${RED}Error: Content-Length is incorrect. Expected: $expected_content_length, Actual: $HEAD_CONTENT_LENGTH${NC}"
                kill $WEBSERV_PID
                return 1
            fi
        else
            # Get response headers for GET request
            GET_RESPONSE_HEADERS=$(curl -s -I "http://localhost:8081$path" 2>&1)
            # Get Content-Length for GET request
            GET_CONTENT_LENGTH=$(echo "$GET_RESPONSE_HEADERS" | grep -i -E "^Content-Length" | awk '{print $2}' | tr -d '\r')
            
            # Check that Content-Length of HEAD and GET are the same
            if [[ "$HEAD_CONTENT_LENGTH" == "$GET_CONTENT_LENGTH" ]]; then
                echo -e "  ${GREEN}Success: Content-Length of HEAD and GET are the same.${NC}"
            else
                echo -e "  ${RED}Error: Content-Length of HEAD and GET are different. HEAD: $HEAD_CONTENT_LENGTH, GET: $GET_CONTENT_LENGTH${NC}"
                kill $WEBSERV_PID
                return 1
            fi
        fi
    fi

    # For redirects, check Location header
    if [[ -n "$expected_location" ]]; then
        LOCATION_HEADER=$(echo "$HEAD_RESPONSE_HEADERS" | grep -i -E "^Location" | awk '{print $2}' | tr -d '\r')
        if [[ "$LOCATION_HEADER" == "$expected_location" ]]; then
            echo -e "  ${GREEN}Success: Location header is correct.${NC}"
        else
            echo -e "  ${RED}Error: Location header is incorrect. Expected: $expected_location, Actual: $LOCATION_HEADER${NC}"
            kill $WEBSERV_PID
            return 1
        fi
    fi


    # Kill the webserv process
    kill $WEBSERV_PID 2>/dev/null
    wait $WEBSERV_PID 2>/dev/null
    sleep 0.5
    echo ""
    return 0
}

# --- Test Cases ---

# 1. HEAD request for a static file
run_test "$PROJECT_ROOT/test/head_test/config.yaml" "HEAD Request" "/hello.txt" "200" "" ""
if [ $? -ne 0 ]; then
    overall_status=1
fi

# 2. HEAD request for a non-existent file
run_test "$PROJECT_ROOT/test/head_test/config.yaml" "HEAD Request for Non-Existent File" "/non-existent-file.txt" "404" "" ""
if [ $? -ne 0 ]; then
    overall_status=1
fi

# 3. HEAD request for a directory with index file
run_test "$PROJECT_ROOT/test/head_test/config.yaml" "HEAD Request for Directory" "/" "200" "" ""
if [ $? -ne 0 ]; then
    overall_status=1
fi

# 4. HEAD request for a location where HEAD is not allowed
run_test "$PROJECT_ROOT/test/head_test/config.yaml" "HEAD Request for Disallowed Method" "/no_head/hello.txt" "405" "" ""
if [ $? -ne 0 ]; then
    overall_status=1
fi

# 5. HEAD request for a directory with autoindex on
run_test "$PROJECT_ROOT/test/head_test/config_autoindex_on.yaml" "HEAD Request for Autoindex On" "/no_autoindex_dir/" "200" "" ""
if [ $? -ne 0 ]; then
    overall_status=1
fi

# 6. HEAD request for a redirect
run_test "$PROJECT_ROOT/test/head_test/config_redirect.yaml" "HEAD Request for Redirect" "/redirect" "301" "http://localhost:8081/hello.txt" ""
if [ $? -ne 0 ]; then
    overall_status=1
fi

# 7. HEAD request for a large file
run_test "$PROJECT_ROOT/test/head_test/config.yaml" "HEAD Request for Large File" "/large_file.txt" "200" "" "10485760"
if [ $? -ne 0 ]; then
    overall_status=1
fi

# 8. HEAD request for a file with no read permissions
run_test "$PROJECT_ROOT/test/head_test/config.yaml" "HEAD Request for No Read File" "/no_read.txt" "403" "" ""
if [ $? -ne 0 ]; then
    overall_status=1
fi

# 9. HEAD request with a body (SKIPPED: curl -I and -d are mutually exclusive)
# run_test "$PROJECT_ROOT/test/head_test/config.yaml" "HEAD Request with Body" "/hello.txt" "200" "" "" "-d 'some body'"
# if [ $? -ne 0 ]; then
#     overall_status=1
# fi
echo "Running test: HEAD Request with Body (Config: $PROJECT_ROOT/test/head_test/config.yaml, Path: /hello.txt)"
echo "  Skipped: This test is not applicable (curl -I and -d are mutually exclusive)"
echo ""

# 10. HEAD request with query parameters
run_test "$PROJECT_ROOT/test/head_test/config.yaml" "HEAD Request with Query" "/hello.txt?a=1&b=2" "200" "" ""
if [ $? -ne 0 ]; then
    overall_status=1
fi


if [ $overall_status -eq 0 ]; then
    echo -e "${GREEN}All HEAD method tests passed!${NC}"
    exit 0
else
    echo -e "${RED}Some HEAD method tests failed!${NC}"
    exit 1
fi