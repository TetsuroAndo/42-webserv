#include "RequestBodyParserTest.hpp"
#include "../../Core/HttpRequest.hpp"
#include "../../Core/HttpStatus.hpp"
#include "../RequestBodyParser.hpp"
#include <iostream>
#include <string>

#define PASS "\033[32m[PASS]\033[0m"
#define FAIL "\033[31m[FAIL]\033[0m"
#define ASSERT(cond, msg)                                                      \
	do {                                                                       \
		if (!(cond)) {                                                         \
			std::cout << FAIL << " " << msg << std::endl;                      \
			return;                                                            \
		}                                                                      \
		std::cout << PASS << " " << msg << std::endl;                          \
	} while (0)

static void testIdentityZeroLength() {
	HttpRequest req;
	req.addHeader("Content-Length", "0");

	RequestBodyParser parser;
	int err = 0;
	parser.init(req, err);
	ASSERT(err == 0, "Identity init zero length");

	ParseResult result;
	size_t consumed = parser.parse(req, "", err, result);
	ASSERT(result == PARSE_COMPLETE, "Parse complete for zero length");
	ASSERT(consumed == 0, "Consumed 0 bytes for zero length");
}

static void testIdentityNormal() {
	HttpRequest req;
	req.addHeader("Content-Length", "5");

	RequestBodyParser parser;
	int err = 0;
	parser.init(req, err);
	ASSERT(err == 0, "Identity init normal");

	ParseResult result;
	size_t consumed = parser.parse(req, "abc", err, result);
	ASSERT(result == PARSE_INCOMPLETE, "Parse incomplete for partial data");
	ASSERT(consumed == 3, "Consumed 3 bytes");

	consumed = parser.parse(req, "de", err, result);
	ASSERT(result == PARSE_COMPLETE, "Parse complete for full data");
	ASSERT(req.getBody() == "abcde", "Body matches 'abcde'");
}

static void testChunkedNormal() {
	HttpRequest req;
	req.addHeader("Transfer-Encoding", "chunked");

	RequestBodyParser parser;
	int err = 0;
	parser.init(req, err);
	ASSERT(err == 0, "Chunked init normal");

	ParseResult result;
	parser.parse(req, "3\r\nabc\r\n2\r\nde\r\n0\r\n\r\n", err, result);
	ASSERT(result == PARSE_COMPLETE, "Chunked parse complete");
	ASSERT(req.getBody() == "abcde", "Chunked body matches 'abcde'");
}

static void testChunkedInvalidSize() {
	HttpRequest req;
	req.addHeader("Transfer-Encoding", "chunked");

	RequestBodyParser parser;
	int err = 0;
	parser.init(req, err);

	ParseResult result;
	parser.parse(req, "Z\r\nabc\r\n0\r\n\r\n", err, result);
	ASSERT(err == HttpStatus::BAD_REQUEST, "Chunked invalid size detected");
}

static void testChunkedMissingCRLF() {
	HttpRequest req;
	req.addHeader("Transfer-Encoding", "chunked");

	RequestBodyParser parser;
	int err = 0;
	parser.init(req, err);

	ParseResult result;
	parser.parse(req, "3\r\nabc0\r\n\r\n", err, result);
	ASSERT(err == HttpStatus::BAD_REQUEST, "Chunked missing CRLF detected");
}

static void testInitErrors() {
	HttpRequest req;
	req.addHeader("Transfer-Encoding", "chunked");
	req.addHeader("Content-Length", "5");

	RequestBodyParser parser;
	int err = 0;
	parser.init(req, err);
	ASSERT(err == HttpStatus::BAD_REQUEST, "Init error for both headers");

	HttpRequest req2;
	req2.addHeader("Transfer-Encoding", "gzip");
	RequestBodyParser parser2;
	err = 0;
	parser2.init(req2, err);
	ASSERT(err == HttpStatus::NOT_IMPLEMENTED,
		   "Init error for unsupported encoding");
}

void runAllRequestBodyParserTests() {
	testIdentityZeroLength();
	testIdentityNormal();
	testChunkedNormal();
	testChunkedInvalidSize();
	testChunkedMissingCRLF();
	testInitErrors();
}
