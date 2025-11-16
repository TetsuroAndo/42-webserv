#include "RequestLineParserTest.hpp"
#include "../../Core/HttpRequest.hpp"
#include "../../Core/HttpStatus.hpp"
#include "../../Parser/RequestLineParser.hpp"
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

static void testValidLines() {
	HttpRequest req;
	RequestLineParser parser;
	int err = 0;

	ASSERT(parser.parse(req, "GET /index.html HTTP/1.1", err) == true,
		   "Standard GET");
	ASSERT(req.getMethod() == "GET", "Method GET");
	ASSERT(req.getPath() == "/index.html", "Path /index.html");
	ASSERT(req.getVersion() == "HTTP/1.1", "Version HTTP/1.1");

	req.clear();
	err = 0;
	ASSERT(parser.parse(req, "POST /api?x=1&y=2 HTTP/1.0", err) == true,
		   "POST with query");
	ASSERT(req.getMethod() == "POST", "Method POST");
	ASSERT(req.getPath() == "/api", "Path /api");
	ASSERT(req.getQuery("x") == "1", "Query x=1");
	ASSERT(req.getQuery("y") == "2", "Query y=2");
}

static void testEmptyAndMalformed() {
	HttpRequest req;
	RequestLineParser parser;
	int err = 0;

	req.clear();
	err = 0;
	ASSERT(parser.parse(req, "", err) == false &&
			   err == HttpStatus::BAD_REQUEST,
		   "Empty line");

	req.clear();
	err = 0;
	ASSERT(parser.parse(req, "GET /index.html", err) == false &&
			   err == HttpStatus::BAD_REQUEST,
		   "Missing version");

	req.clear();
	err = 0;
	ASSERT(parser.parse(req, "GET /index.html HTTP/2.0", err) == false &&
			   err == HttpStatus::VERSION_NOT_SUPPORTED,
		   "Unsupported version");
}

static void testSpecialQueries() {
	HttpRequest req;
	RequestLineParser parser;
	int err = 0;

	req.clear();
	err = 0;
	ASSERT(parser.parse(req, "GET /path?key HTTP/1.1", err) == true,
		   "Query key without value");
	ASSERT(req.getQuery("key") == "", "Query key empty");

	req.clear();
	err = 0;
	ASSERT(parser.parse(req, "GET /path?key= HTTP/1.1", err) == true,
		   "Query key empty value");
	ASSERT(req.getQuery("key") == "", "Query key empty");

	req.clear();
	err = 0;
	ASSERT(parser.parse(req, "GET /path?key=1&key=2 HTTP/1.1", err) == true,
		   "Duplicate keys");
	ASSERT(req.getQuery("key") == "2", "Last value overwrites");
}

void runRequestLineParserTests() {
	testValidLines();
	testEmptyAndMalformed();
	testSpecialQueries();
}
