#include "../ResponseBuilder.hpp"
#include "../../Core/HttpResponse.hpp"
#include "../../Core/HttpStatus.hpp"
#include <cstdlib>
#include <iostream>
#include <string>

#define PASS "\033[32m[PASS]\033[0m"
#define FAIL "\033[31m[FAIL]\033[0m"
#define ASSERT(cond, msg) \
    do { \
        if (!(cond)) { \
            std::cerr << FAIL << " " << msg << std::endl; \
        } else { \
            std::cout << PASS << " " << msg << std::endl; \
        } \
    } while(0)

static void testStatusLineOk() {
    HttpResponse res("TestServer");
    res.setVersion("HTTP/1.1");
    res.setStatusCode(HttpStatus::OK);
    std::string out = ResponseBuilder::build(res);
    ASSERT(out.find("HTTP/1.1 200 OK\r\n") != std::string::npos, "Status line 200 OK");
}

static void testStatusLineNotFound() {
    HttpResponse res("TestServer");
    res.setVersion("HTTP/1.0");
    res.setStatusCode(HttpStatus::NOT_FOUND);
    std::string out = ResponseBuilder::build(res);
    ASSERT(out.find("HTTP/1.0 404 Not Found\r\n") != std::string::npos, "Status line 404 Not Found");
}

static void testStatusLineUnknownCode() {
    HttpResponse res("TestServer");
    res.setVersion("HTTP/1.1");
    res.setStatusCode(999);
    std::string out = ResponseBuilder::build(res);
    ASSERT(out.find("HTTP/1.1 999 Internal Server Error\r\n") != std::string::npos, "Unknown code fallback");
}

static void testDefaultHeaders() {
    HttpResponse res("TestServer");
    res.setVersion("HTTP/1.1");
    res.setStatusCode(HttpStatus::OK);
    res.setBody("hello");
    std::string out = ResponseBuilder::build(res);

    ASSERT(out.find("Date:") != std::string::npos, "Date header");
    ASSERT(out.find("Server:") != std::string::npos, "Server header");
    ASSERT(out.find("Content-Length: 5") != std::string::npos, "Content-Length header");
    ASSERT(out.find("Content-Type: text/html") != std::string::npos, "Content-Type header");
}

static void testUserDefinedHeaders() {
    HttpResponse res("TestServer");
    res.setVersion("HTTP/1.1");
    res.setStatusCode(HttpStatus::OK);
    res.setHeader("Date", "CustomDate");
    res.setHeader("Server", "CustomServer");
    res.setHeader("Content-Length", "1234");
    res.setHeader("Content-Type", "application/json");
    res.setBody("abcd");

    std::string out = ResponseBuilder::build(res);

    ASSERT(out.find("Date: CustomDate") != std::string::npos, "Custom Date kept");
    ASSERT(out.find("Server: CustomServer") != std::string::npos, "Custom Server kept");
    ASSERT(out.find("Content-Length: 1234") != std::string::npos, "Custom Content-Length kept");
    ASSERT(out.find("Content-Type: application/json") != std::string::npos, "Custom Content-Type kept");
}

static void testContentLengthMatchesBody() {
    HttpResponse res("TestServer");
    res.setVersion("HTTP/1.1");
    res.setStatusCode(HttpStatus::OK);
    res.setBody("12345");
    std::string out = ResponseBuilder::build(res);
    ASSERT(out.find("Content-Length: 5") != std::string::npos, "Body length matches Content-Length");
}

static void testBodyOutput() {
    HttpResponse res("TestServer");
    res.setVersion("HTTP/1.1");
    res.setStatusCode(HttpStatus::OK);
    res.setBody("HelloWorld");
    std::string out = ResponseBuilder::build(res);
    ASSERT(out.find("\r\n\r\nHelloWorld") != std::string::npos, "Body correctly placed after headers");
}

static void testEmptyBody() {
    HttpResponse res("TestServer");
    res.setVersion("HTTP/1.1");
    res.setStatusCode(HttpStatus::OK);
    res.setBody("");
    std::string out = ResponseBuilder::build(res);
    ASSERT(out.find("Content-Length: 0") != std::string::npos, "Empty body Content-Length 0");
    ASSERT(out.find("\r\n\r\n") != std::string::npos, "Header-body separator exists");
}

static void testCRLFFormat() {
    HttpResponse res("TestServer");
    res.setVersion("HTTP/1.1");
    res.setStatusCode(HttpStatus::OK);
    res.setBody("X");
    std::string out = ResponseBuilder::build(res);
    size_t pos = out.find("\n");
    ASSERT(pos == std::string::npos || (pos > 0 && out[pos - 1] == '\r'), "CRLF format");
}

void registerHttpResponseBuilderTests() {
    testStatusLineOk();
    testStatusLineNotFound();
    testStatusLineUnknownCode();
    testDefaultHeaders();
    testUserDefinedHeaders();
    testContentLengthMatchesBody();
    testBodyOutput();
    testEmptyBody();
    testCRLFFormat();
}

int main() {
    registerHttpResponseBuilderTests();
    return 0;
}
