#include "HttpResponseTest.hpp"
#include "../HttpResponse.hpp"
#include <iostream>

#define PASS "\033[32m[PASS]\033[0m"
#define FAIL "\033[31m[FAIL]\033[0m"

static void testConstructorAndClear() {
    HttpResponse res("TestServer");
    std::cout << (res.getServerName() == "TestServer" ? PASS : FAIL) << " ServerName default\n";
    std::cout << (res.getStatusCode() == 200 ? PASS : FAIL) << " Status default\n";
    std::cout << (res.getVersion() == HTTP_VERSION ? PASS : FAIL) << " Version default\n";

    res.setStatusCode(404);
    res.setVersion("HTTP/1.1");
    res.setHeader("Content-Type", "text/plain");
    res.setBody("Hello");

    res.clear();
    std::cout << (res.getStatusCode() == 200 ? PASS : FAIL) << " Status after clear\n";
    std::cout << (res.getHeaders().empty() ? PASS : FAIL) << " Headers after clear\n";
    std::cout << (res.getBody().empty() ? PASS : FAIL) << " Body after clear\n";
}

static void testServerName() {
    HttpResponse res("TestServer");
    res.setServerName("MyServer");
    std::cout << (res.getServerName() == "MyServer" ? PASS : FAIL) << " ServerName set/get\n";
}

static void testStatusCode() {
    HttpResponse res("TestServer");
    res.setStatusCode(404);
    std::cout << (res.getStatusCode() == 404 ? PASS : FAIL) << " StatusCode set/get\n";
}

static void testVersion() {
    HttpResponse res("TestServer");
    res.setVersion("HTTP/1.1");
    std::cout << (res.getVersion() == "HTTP/1.1" ? PASS : FAIL) << " Version set/get\n";
}

static void testHeaders() {
    HttpResponse res("TestServer");
    res.setHeader("Content-Type", "text/html");
    std::cout << (res.hasHeader("Content-Type") ? PASS : FAIL) << " hasHeader\n";
    std::cout << (res.getHeader("Content-Type") == "text/html" ? PASS : FAIL) << " getHeader\n";
}

static void testBody() {
    HttpResponse res("TestServer");
    res.setBody("Hello");
    std::cout << (res.getBody() == "Hello" ? PASS : FAIL) << " Body set/get\n";
}

void registerHttpResponseTests() {
    testConstructorAndClear();
    testServerName();
    testStatusCode();
    testVersion();
    testHeaders();
    testBody();
}
