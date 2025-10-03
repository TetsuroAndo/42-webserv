#pragma once
#include <string>
#include <map>

class HttpResponse {
public:
    HttpResponse(const std::string &serverName = "DummyServer")
        : _serverName(serverName), _version("HTTP/1.1"), _statusCode(200) {}

    // --- Version ---
    std::string getVersion() const { return _version; }
    void setVersion(const std::string &v) { _version = v; }

    // --- Status ---
    int getStatusCode() const { return _statusCode; }
    void setStatusCode(int c) { _statusCode = c; }

    // --- Server Name ---
    std::string getServerName() const { return _serverName; }

    // --- Body ---
    void setBody(const std::string &b) { _body = b; }
    std::string getBody() const { return _body; }

    // --- Headers ---
    void setHeader(const std::string &k, const std::string &v) {
        _headers[k] = v;
    }
    bool hasHeader(const std::string &k) const {
        return _headers.find(k) != _headers.end();
    }
    const std::map<std::string,std::string>& getHeaders() const {
        return _headers;
    }

private:
    std::string _serverName;
    std::string _version;
    int _statusCode;
    std::string _body;
    std::map<std::string,std::string> _headers;
};

// --- Minimal HttpStatus Stub ---
struct HttpStatus {
    static std::string getReason(int code) {
        if (code == 200) return "OK";
        if (code == 404) return "Not Found";
        if (code == 500) return "Internal Server Error";
        return "Unknown";
    }
};

// --- Minimal TimeCache Stub ---
struct TimeCache {
    static std::string getCurrentTime() {
        return "Sun, 29 Sep 2025 12:00:00 GMT";
    }
};
