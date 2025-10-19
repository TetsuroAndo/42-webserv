#!/bin/bash

# Get the project root directory (2 levels up from this script)
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"

WEBSERV_BIN="$PROJECT_ROOT/webserv"

# Colors
GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m' # No Color

# Function to sort query parameters
sort_query_params() {
    local query_string=$1
    if [ -z "$query_string" ]; then
        echo ""
        return
    fi
    # Remove leading '?' if present, then split by '&', sort, and re-join with '&'
    echo "$query_string" | sed 's/^\?//' | tr '&' '\n' | sort | tr '\n' '&' | sed 's/&$//'
}

# Function to run a test case
run_test() {
    local config_file=$1
    local test_name=$2
    local path=$3
    local expected_status=$4
    local expected_location=$5

    echo "Running test: $test_name (Config: $config_file, Path: $path)"

    # Start the webserv in the background from project root
    cd "$PROJECT_ROOT"
    $WEBSERV_BIN "$PROJECT_ROOT/$config_file" &
    WEBSERV_PID=$!

    # Give the server a moment to start up
    sleep 1

    # Send a request and capture the headers
    RESPONSE_HEADERS=$(curl -v -s -o /dev/null "http://localhost:8080$path" 2>&1 | grep -i -E "^< (HTTP|Location)")

    # Check for expected status code (HTTP/1.1 or HTTP/1.0)
    if echo "$RESPONSE_HEADERS" | grep -q "< HTTP/1\.[01] $expected_status"; then
        echo -e "  ${GREEN}Success: Status code $expected_status found.${NC}"
    else
        echo -e "  ${RED}Error: Status code $expected_status not found.${NC} Headers: $RESPONSE_HEADERS"
        kill $WEBSERV_PID
        return 1
    fi

    # Check for expected Location header (if applicable)
    if [ -n "$expected_location" ]; then
        ACTUAL_LOCATION=$(echo "$RESPONSE_HEADERS" | grep -i "^< Location:" | sed -E 's/^< Location: (.*)\r$/\1/')

        # Extract and sort query parameters for comparison
        ACTUAL_PATH_PART=$(echo "$ACTUAL_LOCATION" | cut -d'?' -f1)
        ACTUAL_QUERY_PART=$(echo "$ACTUAL_LOCATION" | cut -d'?' -f2-)
        SORTED_ACTUAL_QUERY=$(sort_query_params "$ACTUAL_QUERY_PART")

        EXPECTED_PATH_PART=$(echo "$expected_location" | cut -d'?' -f1)
        EXPECTED_QUERY_PART=$(echo "$expected_location" | cut -d'?' -f2-)
        SORTED_EXPECTED_QUERY=$(sort_query_params "$EXPECTED_QUERY_PART")

        if [[ "$ACTUAL_PATH_PART" == "$EXPECTED_PATH_PART" ]] && \
           [[ "$SORTED_ACTUAL_QUERY" == "$SORTED_EXPECTED_QUERY" ]]; then
            echo -e "  ${GREEN}Success: Location header '$expected_location' found (order-agnostic).${NC}"
        else
            echo -e "  ${RED}Error: Location header mismatch.${NC}"
            echo -e "    Expected Path: '$EXPECTED_PATH_PART', Actual Path: '$ACTUAL_PATH_PART'"
            echo -e "    Expected Query: '$SORTED_EXPECTED_QUERY', Actual Query: '$SORTED_ACTUAL_QUERY'"
            echo -e "    Full Actual Location: '$ACTUAL_LOCATION'"
            kill $WEBSERV_PID
            return 1
        fi
    fi

    # Kill the webserv process
    kill $WEBSERV_PID
    echo ""
    return 0
}

# --- Test Cases ---

# 1. Basic 301 Redirect
run_test "test/redirect_test/config_301.yaml" "Basic 301" "/old-path" "301 Moved Permanently" "/new-path" || exit 1

# 2. Basic 302 Redirect
run_test "test/redirect_test/config_302.yaml" "Basic 302" "/temp-old" "302 Found" "/temp-new" || exit 1

# 3. Prefix Matching (Longest Match)
echo ""
echo "--- Advanced Feature Tests ---"
run_test "test/redirect_test/config_prefix.yaml" "Prefix Longest Match" "/prefix/path/resource" "301 Moved Permanently" "/new-prefix/path-specific/resource" || exit 1
run_test "test/redirect_test/config_prefix.yaml" "Prefix Shorter Match" "/prefix/resource" "301 Moved Permanently" "/new-prefix/resource" || exit 1

# 4. External Redirect
run_test "test/redirect_test/config_external.yaml" "External Redirect" "/external" "302 Found" "http://example.com" || exit 1

# 5. Redirect with Query Parameters
run_test "test/redirect_test/config_query.yaml" "Query Params Redirect" "/query?param=value&another=test" "307 Temporary Redirect" "/new-query?param=value&another=test" || exit 1

echo ""
echo "✅ All redirect tests passed!"
exit 0