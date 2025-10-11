#!/bin/bash

# Exit on error
set -e

# --- Colors ---
GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m' # No Color

# --- Test Setup ---
TEST_DIR=$(dirname "$0")
LOG_DIR="../../logs"
LOG_FILE="rotation_test.log"
CONFIG_FILE="config.yaml"

# --- Main ---
cd "$TEST_DIR"

echo "--- Preparing Environment ---"
mkdir -p "$LOG_DIR"
rm -f "$LOG_DIR/$LOG_FILE"*

echo "--- Building Test Program ---"
make fclean
make

echo -e "\n--- Running Log Rotation Test (Simple) ---"
./log_rotation_test_app "$CONFIG_FILE"
echo "Test program finished."


# --- Verification ---
echo -e "\n--- Verifying Rotated Logs ---"
ls -l "$LOG_DIR"

LOG1="$LOG_DIR/$LOG_FILE.1"
CURRENT_LOG="$LOG_DIR/$LOG_FILE"

# 1. Verify that rotation happened and files were created
if [ -f "$LOG1" ] && [ -f "$CURRENT_LOG" ]; then
    echo -e "${GREEN}SUCCESS: Both rotated log '$LOG1' and current log '$CURRENT_LOG' exist.${NC}"
else
    echo -e "${RED}FAILURE: Log files were not created as expected.${NC}"
    exit 1
fi

# 2. Verify total line count
echo "Verifying total line count..."
total_lines=$(cat "$LOG1" "$CURRENT_LOG" | grep -c "/rotation-test-path")

if [ "$total_lines" -eq 12 ]; then
    echo -e "${GREEN}SUCCESS: Total of 12 log entries found across rotated files.${NC}"
else
    echo -e "${RED}FAILURE: Found $total_lines total entries, expected 12.${NC}"
    exit 1
fi

# 3. Verify line count in each file
lines_in_backup=$(grep -c "/rotation-test-path" "$LOG1")
lines_in_current=$(grep -c "/rotation-test-path" "$CURRENT_LOG")
echo "Found $lines_in_backup lines in backup and $lines_in_current lines in current log."

if [ "$lines_in_backup" -gt 0 ] && [ "$lines_in_current" -gt 0 ]; then
    echo -e "${GREEN}SUCCESS: Logs were correctly split across two files.${NC}"
else
    echo -e "${RED}FAILURE: Logs were not split as expected.${NC}"
    exit 1
fi

# 4. Verify that no other backup files exist
if [ -f "$LOG_DIR/$LOG_FILE.2" ]; then
    echo -e "${RED}FAILURE: Unexpected backup file '$LOG_DIR/$LOG_FILE.2' was found.${NC}"
    exit 1
else
    echo -e "${GREEN}SUCCESS: No unexpected backup files were found.${NC}"
fi


# --- Cleanup ---
echo -e "\n--- Cleaning Up Build Files ---"
make fclean

echo -e "\n${GREEN}SIMPLE LOG ROTATION TEST PASSED${NC}"