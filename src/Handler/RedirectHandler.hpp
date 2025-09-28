#pragma once

#include "../Config/Config.hpp"
#include "../Http/Core/HttpRequest.hpp"
#include "../Http/Core/HttpResponse.hpp"
#include "ISubHandler.hpp"

class RedirectHandler : public ISubHandler {
public:
    RedirectHandler(const std::string& redirectUrl, int statusCode);
    ~RedirectHandler();

    HttpResponse handle(const HttpRequest& req, const Config& config);

private:
    std::string _redirectUrl;
    int _statusCode;

    RedirectHandler(const RedirectHandler& other);
    RedirectHandler& operator=(const RedirectHandler& other);
};
