#!/bin/bash

WEBSERV_BIN="./webserv"
CONFIG_FILE="./test/confs/valid/post_test.yaml"
UPLOAD_DIR="./test/post_test/uploads"

# Colors
GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m' # No Color

# Cleanup function
cleanup() {
    rm -rf "$UPLOAD_DIR" 2>/dev/null || true
}

trap cleanup EXIT

# Ensure upload dir exists and is clean
rm -rf "$UPLOAD_DIR" 2>/dev/null || true
mkdir -p "$UPLOAD_DIR"

run_post_test() {
    local file_path=$1
    local expected_status=$2
    local should_create=$3 # 1 = expect file created, 0 = expect rejection

    echo "Running POST test with: $file_path (expect $expected_status)"

    # Start server
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

# Files sized 1, 2, 3 bytes respectively
run_post_test "test/test_assets/size1.txt" "201" 1
run_post_test "test/test_assets/size2.txt" "201" 1
run_post_test "test/test_assets/size3.txt" "201" 1

echo -e "${GREEN}All POST size limit tests passed!${NC}"
exit 0
