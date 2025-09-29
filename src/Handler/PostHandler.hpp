#pragma once

#include "ISubHandler.hpp"

class PostHandler : public ISubHandler
{
public:
    PostHandler();
    ~PostHandler();

    HttpResponse handle(const HttpRequest& req, const Config& config);

private:
};
