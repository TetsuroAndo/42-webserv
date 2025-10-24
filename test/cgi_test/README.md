# CGI Test

This directory contains tests for CGI (Common Gateway Interface) functionality.

## Structure

- `run.sh` - Test runner script that tests CGI execution
- `config.yaml` - Configuration file for CGI tests
- `../../cgi-bin/` - Directory containing CGI scripts used for testing

## Running Tests

```bash
./test/cgi_test/run.sh
```

## Test Cases

The test suite includes:
1. Simple CGI GET request
2. Echo CGI with GET (no data)
3. Echo CGI with POST data
4. Echo CGI with query string
5. Non-existent CGI script (404 test)

## Output Format

The test follows the same color-coded output format as other tests:
- GREEN: Success messages
- RED: Failure messages  
- BLUE: Informational messages

