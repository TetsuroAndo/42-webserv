#!/bin/bash

# --- Config ---
GREEN=$(printf '\033[0;32m')
RED=$(printf '\033[0;31m')
BLUE=$(printf '\033[0;34m')
NC=$(printf '\033[0m')

cd "$(dirname "$0")/../.."
PROJECT_ROOT=$(pwd)

CGI_UNIT_TEST="$PROJECT_ROOT/test/cgi_unit_test"

echo -e "${BLUE}Running CGI unit tests...${NC}\n"

if [ ! -f "$CGI_UNIT_TEST" ]; then
    echo -e "${RED}FAIL: CGI unit test executable not found at $CGI_UNIT_TEST${NC}"
    exit 1
fi

# Run the CGI unit test
OUTPUT=$("$CGI_UNIT_TEST" 2>&1)
EXIT_CODE=$?

echo "$OUTPUT"

if [ $EXIT_CODE -eq 0 ]; then
    echo -e "\n${GREEN}SUCCESS: All CGI tests passed!${NC}"
    exit 0
else
    echo -e "\n${RED}FAIL: CGI tests failed with exit code $EXIT_CODE${NC}"
    exit 1
fi
