#pragma once
#include "../../Http/Core/HttpRequest.hpp"
#include <map>
#include <string>

class DummyHttpReq : public HttpRequest {
public:
	DummyHttpReq(const std::string& method, const std::string& path,
					 const std::map<std::string, std::string>& headers,
					 const std::string& body,
					 const std::map<std::string, std::string>& queries);
	
	const std::string& getMethod() const;
	const std::string& getPath() const;
	const std::string& getHeader(const std::string& key) const;
	const std::string& getBody() const;
	const std::map<std::string, std::string>& getQueries() const;

private:
	std::string _method;
	std::string _path;
	std::map<std::string, std::string> _headers;
	std::string _body;
	std::map<std::string, std::string> _queries;
};
