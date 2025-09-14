#include <cassert>
#include <iostream>
#include <sstream>

#include "../HttpRequest.hpp"

namespace URI {
std::string decodeURIComponent(const std::string &encoded) { return encoded; }
} // namespace URI

static void runTest(const std::string &testName, bool result) {
	std::cout << "Test: " << testName << "... ";
	if (result) {
		std::cout << "SUCCESS" << std::endl;
	} else {
		std::cout << "FAILURE" << std::endl;
		assert(false);
	}
}

static void validTestGetMethod() {
	HttpRequest req;
	std::string buffer =
		"GET /index.html?name=test HTTP/1.1\r\nHost: example.com\r\n\r\n";
	ParseStatus st = req.parse(buffer);
	runTest("Valid GET Request", st == PARSE_COMPLETE && req.isComplete() &&
									 req.getMethod() == "GET" &&
									 req.getPath() == "/index.html" &&
									 req.getVersion() == "HTTP/1.1" &&
									 req.getHeader("host") == "example.com");
	req.printData();
}

static void validTestPostMethod() {
	HttpRequest req;
	std::string buffer =
		"POST /api/data HTTP/1.1\r\nContent-Length: 13\r\n\r\nHello, World!";
	ParseStatus st = req.parse(buffer);
	runTest("Valid POST Request (Content-Length)",
			st == PARSE_COMPLETE && req.isComplete() &&
				req.getMethod() == "POST" && req.getBody() == "Hello, World!");
	req.printData();
}

static void validTestDeleteMethod() {
	HttpRequest req;
	std::string buffer =
		"DELETE /resource/1 HTTP/1.1\r\nHost: example.com\r\n\r\n";
	ParseStatus st = req.parse(buffer);
	runTest("DELETE Method", st == PARSE_COMPLETE && req.isComplete() &&
								 req.getMethod() == "DELETE");
	req.printData();
}

static void validTestPostMethodChunked() {
	HttpRequest req;
	std::string buffer =
		"POST /api/data HTTP/1.1\r\nTransfer-Encoding: chunked\r\n\r\n"
		"4\r\nWiki\r\n"
		"5\r\npedia\r\n"
		"E\r\n in\r\n\r\nchunks.\r\n"
		"0\r\n\r\n";
	ParseStatus st = req.parse(buffer);
	runTest("Valid POST Request (Chunked)",
			st == PARSE_COMPLETE && req.isComplete() &&
				req.getMethod() == "POST" &&
				req.getBody() == "Wikipedia in\r\n\r\nchunks.");
	req.printData();
}

static void validTestQuery() {
	HttpRequest req;
	std::string buffer = "GET /search?q=hello%20world&lang=en "
						 "HTTP/1.1\r\nHost: example.com\r\n\r\n";
	ParseStatus st = req.parse(buffer);
	runTest("Query String Parsing", st == PARSE_COMPLETE && req.isComplete() &&
										req.getPath() == "/search" &&
										req.getHeader("host") == "example.com");
	req.printData();
}

static void invalidTestIncompleteRequest() {
	HttpRequest req;
	std::string buffer = "GET /index.html HTTP/1.1\r\n";
	ParseStatus st = req.parse(buffer);
	runTest("Incomplete Request", st == PARSE_INCOMPLETE && !req.isComplete());
	req.printData();
}

static void invalidTestUnsupportedMethod() {
	HttpRequest req;
	std::string buffer =
		"PUT /resource/1 HTTP/1.1\r\nHost: example.com\r\n\r\n";
	ParseStatus st = req.parse(buffer);
	runTest("Unsupported Method",
			st == PARSE_ERROR && req.getError() == PARSE_ERROR_HTTP_METHOD);
	req.printData();
}

static void invalidTestHttpVersion() {
	HttpRequest req;
	std::string buffer = "GET / HTTP/2.0\r\nHost: example.com\r\n\r\n";
	ParseStatus st = req.parse(buffer);
	runTest("Invalid HTTP Version",
			st == PARSE_ERROR && req.getError() == PARSE_ERROR_HTTP_VERSION);
	req.printData();
}

static void invalidTestHeaderTooLarge() {
	HttpRequest req;
	std::string longHeader(9000, 'A');
	std::string buffer =
		"GET / HTTP/1.1\r\nX-Long-Header: " + longHeader + "\r\n\r\n";
	ParseStatus st = req.parse(buffer);
	runTest("Header Too Large",
			st == PARSE_ERROR && req.getError() == PARSE_ERROR_LARGE_HEADER);
	req.printData();
}

static void invalidTestContentLengthTooLarge() {
	HttpRequest req;
	std::string largeBody(10 * 1024 * 1024 + 1, 'A');
	std::ostringstream oss;
	oss << largeBody.size();
	std::string str = oss.str();
	std::string buffer =
		"POST / HTTP/1.1\r\nContent-Length: " + str + "\r\n\r\n" + largeBody;
	ParseStatus st = req.parse(buffer);
	runTest("Content-Length Too Large",
			st == PARSE_ERROR && req.getError() == PARSE_ERROR_LARGE_REQUEST);
	req.printData();
}

static void invalidTestRequestLine() {
	HttpRequest req1;
	std::string buffer1 =
		"GET/index.html HTTP/1.1\r\nHost: example.com\r\n\r\n";
	runTest("Invalid Request Line1",
			req1.parse(buffer1) == PARSE_ERROR &&
				req1.getError() == PARSE_ERROR_INVALID_REQUEST);
	req1.printData();

	HttpRequest req2;
	std::string buffer2 =
		"GET /index.html HTTP /1.1\r\nHost: example.com\r\n\r\n";
	runTest("Invalid Request Line2",
			req2.parse(buffer2) == PARSE_ERROR &&
				req2.getError() == PARSE_ERROR_INVALID_REQUEST);
	req2.printData();
}

static void invalidTestContentLength() {
	HttpRequest req;
	std::string buffer =
		"POST /api/data HTTP/1.1\r\nContent-Length: abc\r\n\r\nbody";
	runTest("Invalid Content-Length",
			req.parse(buffer) == PARSE_ERROR &&
				req.getError() == PARSE_ERROR_LARGE_REQUEST);
	req.printData();
}

static void invalidTestDuplicateHeader() {
	HttpRequest req;
	std::string buffer =
		"GET / HTTP/1.1\r\nHost: example.com\r\nHost: another.com\r\n\r\n";
	runTest("Duplicate Header",
			req.parse(buffer) == PARSE_ERROR &&
				req.getError() == PARSE_ERROR_INVALID_REQUEST);
	req.printData();
}

static void invalidTestChunkedAndContentLength() {
	HttpRequest req;
	std::string buffer = "POST /data HTTP/1.1\r\nTransfer-Encoding: "
						 "chunked\r\nContent-Length: 5\r\n\r\n";
	ParseStatus st = req.parse(buffer);
	runTest("Chunked + Content-Length Error",
			st == PARSE_ERROR && req.getError() == PARSE_ERROR_INVALID_REQUEST);
	req.printData();
}

static void invalidTestChunkSize() {
	HttpRequest req;
	std::string buffer = "POST /data HTTP/1.1\r\nTransfer-Encoding: "
						 "chunked\r\n\r\nZZ\r\nabcdef\r\n0\r\n\r\n";
	ParseStatus st = req.parse(buffer);
	runTest("Invalid Chunk Size",
			st == PARSE_ERROR && req.getError() == PARSE_ERROR_INVALID_REQUEST);
	req.printData();
}

int main(void) {
	// 正常系
	validTestGetMethod();
	validTestPostMethod();
	validTestDeleteMethod();
	validTestPostMethodChunked();
	validTestQuery();

	// 異常系
	invalidTestIncompleteRequest();

	invalidTestUnsupportedMethod();
	invalidTestHttpVersion();

	invalidTestHeaderTooLarge();
	invalidTestContentLengthTooLarge();

	invalidTestRequestLine();
	invalidTestContentLength();
	invalidTestDuplicateHeader();
	invalidTestChunkedAndContentLength();
	invalidTestChunkSize();

	std::cout << "All tests passed!" << std::endl;
	return 0;
}
