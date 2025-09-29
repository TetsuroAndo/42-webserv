#include "HttpRequestTest.hpp"
#include "../HttpRequest.hpp"
#include <iostream>

#define PASS "\033[32m[PASS]\033[0m"
#define FAIL "\033[31m[FAIL]\033[0m"
#define ASSERT(cond, message) \
    do { \
        if (!(cond)) { \
            std::cout << FAIL << " " << message << std::endl; \
			return; \
        } \
            std::cout << PASS << " " << message << std::endl; \
    } while(0)

static void testConstructorAndClear() {
    HttpRequest req;
    ASSERT(req.getMethod().empty() && req.getPath().empty() && req.getVersion().empty() &&
           req.getHeaders().empty() && req.getQueries().empty() && req.getBody().empty(),
           "HttpRequest constructor defaults");

    req.setMethod("POST");
    req.setPath("/test");
    req.setVersion("HTTP/1.1");
    req.addHeader("Host", "example.com");
    req.addQuery("q", "1");
    req.setBody("body");
    req.clear();

    ASSERT(req.getMethod().empty() && req.getPath().empty() && req.getVersion().empty() &&
           req.getHeaders().empty() && req.getQueries().empty() && req.getBody().empty(),
           "HttpRequest clear");
}

static void testMethodPathVersion() {
    HttpRequest req;
    req.setMethod("GET");
    req.setPath("/index.html");
    req.setVersion("HTTP/1.0");

    ASSERT(req.getMethod() == "GET", "Method set/get");
    ASSERT(req.getPath() == "/index.html", "Path set/get");
    ASSERT(req.getVersion() == "HTTP/1.0", "Version set/get");
}

static void testHeaders() {
    HttpRequest req;
    req.addHeader("Content-Type", "text/plain");
    ASSERT(req.hasHeader("Content-Type"), "hasHeader");
    ASSERT(req.getHeader("Content-Type") == "text/plain", "getHeader");
}

static void testQueries() {
    HttpRequest req;
    req.addQuery("key", "value");
    ASSERT(req.hasQuery("key"), "hasQuery");
    ASSERT(req.getQuery("key") == "value", "getQuery");
}

static void testBody() {
    HttpRequest req;
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
