#include <cassert>
#include <iostream>

#include "../HttpRequest.hpp"

namespace URI {
    std::string decodeURIComponent(const std::string& encoded) {
        return encoded;
    }
}

static void run_test(const std::string& test_name, bool result) {
    std::cout << "Test: " << test_name << "... ";
    if (result) {
        std::cout << "SUCCESS" << std::endl;
    } else {
        std::cout << "FAILURE" << std::endl;
        assert(false);
    }
}

static void test_valid_get_request() {
    HttpRequest req;
    std::string buffer = "GET /index.html?name=test HTTP/1.1\r\nHost: example.com\r\n\r\n";
    run_test("Valid GET Request", req.parse(buffer) && req.isComplete() &&
             req.getMethod() == "GET" && req.getPath() == "/index.html" &&
             req.getVersion() == "HTTP/1.1" && req.getHeader("host") == "example.com");
	req.printData();
}

static void test_valid_post_request_content_length() {
    HttpRequest req;
    std::string buffer = "POST /api/data HTTP/1.1\r\nContent-Length: 13\r\n\r\nHello, World!";
    run_test("Valid POST Request (Content-Length)", req.parse(buffer) && req.isComplete() &&
             req.getMethod() == "POST" && req.getBody() == "Hello, World!");
	req.printData();
}

static void test_valid_post_request_chunked() {
    HttpRequest req;
    std::string buffer = "POST /api/data HTTP/1.1\r\nTransfer-Encoding: chunked\r\n\r\n4\r\nWiki\r\n5\r\npedia\r\nE\r\n in\r\n\r\nchunks.\r\n0\r\n\r\n";
    run_test("Valid POST Request (Chunked)", req.parse(buffer) && req.isComplete() &&
             req.getMethod() == "POST" && req.getBody() == "Wikipedia in\r\n\r\nchunks.");
	req.printData();
}

static void test_incomplete_request() {
    HttpRequest req;
    std::string buffer = "GET /index.html HTTP/1.1\r\n";
    run_test("Incomplete Request", !req.parse(buffer) && !req.isComplete());
}

static void test_header_too_large() {
    HttpRequest req;
    std::string long_header(9000, 'A');
    std::string buffer = "GET / HTTP/1.1\r\nX-Long-Header: " + long_header + "\r\n\r\n";
    run_test("Header Too Large", !req.parse(buffer));
}

static void test_invalid_request_line() {
    HttpRequest req1;
    std::string buffer1 = "GET/index.html HTTP/1.1\r\nHost: example.com\r\n\r\n";
    run_test("Invalid Request Line1", !req1.parse(buffer1));

    HttpRequest req2;
    std::string buffer2 = "GET /index.html HTTP /1.1\r\nHost: example.com\r\n\r\n";
    run_test("Invalid Request Line2", !req2.parse(buffer2));
}

static void test_invalid_content_length() {
    HttpRequest req;
    std::string buffer = "POST /api/data HTTP/1.1\r\nContent-Length: abc\r\n\r\nbody";
    run_test("Invalid Content-Length", !req.parse(buffer));
}

static void test_duplicate_header() {
    HttpRequest req;
    std::string buffer = "GET / HTTP/1.1\r\nHost: example.com\r\nHost: another.com\r\n\r\n";
    run_test("Duplicate Header", !req.parse(buffer));
}

int main() {
    test_valid_get_request();
    test_valid_post_request_content_length();
    test_valid_post_request_chunked();
    test_incomplete_request();
    test_header_too_large();
    test_invalid_request_line();
    test_invalid_content_length();
    test_duplicate_header();

    std::cout << "All tests passed!" << std::endl;
    return 0;
}
