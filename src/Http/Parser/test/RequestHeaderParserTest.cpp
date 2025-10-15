#include "../RequestHeaderParser.hpp"
#include "../../Core/HttpRequest.hpp"
#include "../../Core/HttpStatus.hpp"
#include "../ParseResult.hpp"
#include <iostream>

#define PASS "\033[32m[PASS]\033[0m"
#define FAIL "\033[31m[FAIL]\033[0m"
#define ASSERT(cond, message)                                                  \
	do {                                                                       \
		if (!(cond)) {                                                         \
			std::cout << FAIL << " " << message << std::endl;                  \
			return;                                                            \
		}                                                                      \
		std::cout << PASS << " " << message << std::endl;                      \
	} while (0)

static void testBasicHeaders() {
	HttpRequest req;
	RequestHeaderParser parser;
	int err = 0;

	std::string headers = "Host: example.com\r\n"
						  "Content-Type: text/plain\r\n"
						  "X-Test: 123\r\n";

	ParseResult res = parser.parse(req, headers, err);
	ASSERT(res == PARSE_COMPLETE, "Basic headers parsed");
	ASSERT(req.hasHeader("Host") && req.getHeader("Host") == "example.com",
		   "Host header");
	ASSERT(req.hasHeader("Content-Type") &&
			   req.getHeader("Content-Type") == "text/plain",
		   "Content-Type header");
	ASSERT(req.hasHeader("X-Test") && req.getHeader("X-Test") == "123",
		   "X-Test header");
}

static void testHeaderWhitespace() {
	HttpRequest req;
	RequestHeaderParser parser;
	int err = 0;

	std::string headers = "  Foo-Bar \t :  value \t \r\n";
	ParseResult res = parser.parse(req, headers, err);
	ASSERT(res == PARSE_COMPLETE, "Header with whitespace parsed");
	ASSERT(req.hasHeader("Foo-Bar") && req.getHeader("Foo-Bar") == "value",
		   "Whitespace trimmed");
}

static void testDuplicateHeader() {
	HttpRequest req;
	RequestHeaderParser parser;
	int err = 0;

	std::string headers = "X-Dup: one\r\nX-Dup: two\r\n";
	ParseResult res = parser.parse(req, headers, err);
	ASSERT(res == PARSE_ERROR, "Duplicate header detected");
	ASSERT(err == HttpStatus::BAD_REQUEST, "Duplicate header sets BAD_REQUEST");
}

static void testInvalidHeader() {
	HttpRequest req;
	RequestHeaderParser parser;
	int err = 0;

	std::string headers = "InvalidHeaderWithoutColon\r\n";
	ParseResult res = parser.parse(req, headers, err);
	ASSERT(res == PARSE_ERROR, "Header without colon detected");
	ASSERT(err == HttpStatus::BAD_REQUEST, "Invalid header sets BAD_REQUEST");
}

static void testEmptyHeaderValue() {
	HttpRequest req;
	RequestHeaderParser parser;
	int err = 0;

	std::string headers = "X-Empty: \r\n";
	ParseResult res = parser.parse(req, headers, err);
	ASSERT(res == PARSE_COMPLETE, "Header with empty value allowed");
	ASSERT(req.hasHeader("X-Empty") && req.getHeader("X-Empty").empty(),
		   "Empty value stored");
}

void runRequestHeaderParserTests() {
	testBasicHeaders();
	testHeaderWhitespace();
	testDuplicateHeader();
	testInvalidHeader();
	testEmptyHeaderValue();
}
