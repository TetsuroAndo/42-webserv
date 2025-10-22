#!/bin/bash

# Color codes for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Counters
total_tests=0
passed_tests=0
failed_tests=0

echo "=================================================="
echo "Running All Tests"
echo "=================================================="
echo ""

# Find all run.sh scripts in test subdirectories
for test_script in $(find test/ -mindepth 2 -name "run.sh" -type f | sort); do
	test_name=$(dirname "$test_script" | xargs basename)
	total_tests=$((total_tests + 1))
	
	echo ""
	echo "=================================================="
	echo "Running: $test_name"
	echo "=================================================="
	
	# Run the test script and capture exit code
	bash "$test_script" > /tmp/test_output_$$.log 2>&1
	test_exit_code=$?
	
	# Display the output
	cat /tmp/test_output_$$.log
	rm -f /tmp/test_output_$$.log
	
	# Check exit code (0 or 143 are success, 143 is SIGTERM which is ok for cleanup)
	if [ $test_exit_code -eq 0 ] || [ $test_exit_code -eq 143 ]; then
		echo -e "${GREEN}✓ $test_name PASSED${NC}"
		passed_tests=$((passed_tests + 1))
	else
		echo -e "${RED}✗ $test_name FAILED (exit code: $test_exit_code)${NC}"
		failed_tests=$((failed_tests + 1))
	fi
done

# Summary
echo ""
echo "=================================================="
echo "Test Summary"
echo "=================================================="
echo "Total tests: $total_tests"
echo -e "${GREEN}Passed: $passed_tests${NC}"
echo -e "${RED}Failed: $failed_tests${NC}"
echo "=================================================="

# Exit with failure if any test failed
if [ $failed_tests -gt 0 ]; then
	exit 1
else
	exit 0
fi
