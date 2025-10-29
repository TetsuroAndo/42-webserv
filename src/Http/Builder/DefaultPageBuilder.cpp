#include "DefaultPageBuilder.hpp"

void DefaultPageBuilder::generateSimpleBody(const std::string &method,
											HttpResponse &res, const int code,
											const std::string &description) {
	res.setStatusCode(code);
	const std::string &reason = HttpStatus::getReason(code);
	std::string body;
	body += "<html><head><title>";
	body += StringOps::toString(code);
	body += " ";
	body += reason;
	body += "</title></head><body><h1>";
	body += StringOps::toString(code);
	body += " ";
	body += reason;
	body += "</h1>";
	if (description.empty() == false) {
		body += "<p>";
		body += description;
		body += "</p>";
	}
	body += "</body></html>";
	res.setBody(body);
	res.setHeader("Content-Type", "text/html");
	if (method == "HEAD") {
		res.setBody("");
	} else {
		res.setBody(body);
	}
}
