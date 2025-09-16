#pragma once

#include <string>

class ResponseParser {
public:
	static std::string getStatusMessage(int statusCode);

private:
	ResponseParser();
	ResponseParser(const ResponseParser &o);
	ResponseParser &operator=(const ResponseParser &o);
	~ResponseParser();
};
