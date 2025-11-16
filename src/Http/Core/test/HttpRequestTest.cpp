#include "HttpRequestTest.hpp"
#include "../../../Config/ConfigBuilder.hpp"
#include "../HttpRequest.hpp"
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

static void testConstructorAndClear() {
	ConfigBuilder builder("../../../../config/default.yaml");
	Config config = builder.build();
	HttpRequest req(config);
	ASSERT(req.getMethod().empty() && req.getPath().empty() &&
			   req.getVersion().empty() && req.getHeaders().empty() &&
			   req.getQueries().empty() && req.getBody().empty(),
		   "HttpRequest constructor defaults");

	req.setMethod("POST");
	req.setPath("/test");
	req.setVersion("HTTP/1.1");
	req.addHeader("Host", "example.com");
	req.addQuery("q", "1");
	req.setBody("body");
	req.clear(config);

	ASSERT(req.getMethod().empty() && req.getPath().empty() &&
			   req.getVersion().empty() && req.getHeaders().empty() &&
			   req.getQueries().empty() && req.getBody().empty(),
		   "HttpRequest clear");
}

static void testMethodPathVersion() {
	ConfigBuilder builder("../../../../config/default.yaml");
	Config config = builder.build();
	HttpRequest req(config);
	req.setMethod("GET");
	req.setPath("/index.html");
	req.setVersion("HTTP/1.0");

	ASSERT(req.getMethod() == "GET", "Method set/get");
	ASSERT(req.getPath() == "/index.html", "Path set/get");
	ASSERT(req.getVersion() == "HTTP/1.0", "Version set/get");
}

static void testHeaders() {
	ConfigBuilder builder("../../../../config/default.yaml");
	Config config = builder.build();
	HttpRequest req(config);
	req.addHeader("Content-Type", "text/plain");
	ASSERT(req.hasHeader("Content-Type"), "hasHeader");
	ASSERT(req.getHeader("Content-Type") == "text/plain", "getHeader");
}

static void testQueries() {
	ConfigBuilder builder("../../../../config/default.yaml");
	Config config = builder.build();
	HttpRequest req(config);
	req.addQuery("key", "value");
	ASSERT(req.hasQuery("key"), "hasQuery");
	ASSERT(req.getQuery("key") == "value", "getQuery");
}

static void testBody() {
	ConfigBuilder builder("../../../../config/default.yaml");
	Config config = builder.build();
	HttpRequest req(config);
	req.setBody("hello");
	req.appendBody(", world");
	ASSERT(req.getBody() == "hello, world", "body append");
}

void registerHttpRequestTests() {
	testConstructorAndClear();
	testMethodPathVersion();
	testHeaders();
	testQueries();
	testBody();
}
