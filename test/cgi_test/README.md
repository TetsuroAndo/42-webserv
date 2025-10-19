# CGI Test

This directory contains unit tests for CGI components (CgiEnvBuilder and CgiResponseParser).

## Structure

- `run.sh` - Test runner script that executes the CGI unit test binary
- `../cgi_unit_test` - Binary executable containing unit tests

## Running Tests

```bash
./test/cgi_test/run.sh
```

## Output Format

The test follows the same color-coded output format as other tests:
- GREEN: Success messages
- RED: Failure messages  
- BLUE: Informational messages
