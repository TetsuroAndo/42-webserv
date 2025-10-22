#!/bin/bash

# Get the project root directory (2 levels up from this script)
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"

WEBSERV_BIN="$PROJECT_ROOT/webserv"
CONFIG_FILE="$PROJECT_ROOT/test/post_test/post_test.yaml"
UPLOAD_DIR="$PROJECT_ROOT/test/post_test/uploads"

# Colors
GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m' # No Color

# Ensure upload dir exists and is clean
mkdir -p "$UPLOAD_DIR"

run_post_test() {
    local file_path=$1
    local expected_status=$2
    local should_create=$3 # 1 = expect file created, 0 = expect rejection

    echo "Running POST test with: $file_path (expect $expected_status)"

    # Start server from project root
    cd "$PROJECT_ROOT"
    $WEBSERV_BIN "$CONFIG_FILE" &
    WEBSERV_PID=$!
    sleep 1

    # Count files before
    local before_count
    before_count=$(find "$UPLOAD_DIR" -type f 2>/dev/null | wc -l | tr -d ' ')

    # Perform POST with explicit Content-Type
    local HTTP_STATUS
    HTTP_STATUS=$(curl -s -o /dev/null -w '%{http_code}' -X POST \
        -H 'Content-Type: text/plain' \
        --data-binary "@${file_path}" \
        "http://localhost:8080/upload")

    # Count files after
    local after_count
    after_count=$(find "$UPLOAD_DIR" -type f 2>/dev/null | wc -l | tr -d ' ')

    # Validate status code
    if [[ "$HTTP_STATUS" == "$expected_status" ]]; then
        echo -e "  ${GREEN}OK: Status $HTTP_STATUS as expected.${NC}"
    else
        echo -e "  ${RED}FAIL: Expected status $expected_status, got $HTTP_STATUS.${NC}"
        kill $WEBSERV_PID
        exit 1
    fi

    # Validate creation/rejection
    if [[ "$should_create" == "1" ]]; then
        if [[ $after_count -eq $((before_count + 1)) ]]; then
            echo -e "  ${GREEN}OK: File created in upload store.${NC}"
            # Cleanup created file to keep env tidy (remove newest file)
            newest_file=$(ls -1t "$UPLOAD_DIR" 2>/dev/null | head -n 1)
            if [[ -n "$newest_file" ]]; then
                rm -f "$UPLOAD_DIR/$newest_file"
            fi
        else
            echo -e "  ${RED}FAIL: Expected a new file, counts before=$before_count after=$after_count.${NC}"
            kill $WEBSERV_PID
            exit 1
        fi
    else
        if [[ $after_count -eq $before_count ]]; then
            echo -e "  ${GREEN}OK: No file created (rejected).${NC}"
        else
            echo -e "  ${RED}FAIL: Should not create file, counts before=$before_count after=$after_count.${NC}"
            # Clean unexpected files
            rm -f "$UPLOAD_DIR"/* 2>/dev/null || true
            kill $WEBSERV_PID
            exit 1
        fi
    fi

    # Stop server
    kill $WEBSERV_PID
    echo ""
}

# Clean any leftovers before starting
rm -f "$UPLOAD_DIR"/* 2>/dev/null || true

# Create temporary test files (1, 2, 3 bytes)
TMP_DIR="$PROJECT_ROOT/test/post_test/tmp"
mkdir -p "$TMP_DIR"
echo -n "a" > "$TMP_DIR/size1.txt"
echo -n "ab" > "$TMP_DIR/size2.txt"
echo -n "abc" > "$TMP_DIR/size3.txt"

# Files sized 1, 2, 3 bytes respectively
run_post_test "$TMP_DIR/size1.txt" "201" 1
run_post_test "$TMP_DIR/size2.txt" "201" 1
run_post_test "$TMP_DIR/size3.txt" "413" 0

# Clean up temporary files
rm -rf "$TMP_DIR"

echo -e "${GREEN}All POST size limit tests passed!${NC}"
exit 0

