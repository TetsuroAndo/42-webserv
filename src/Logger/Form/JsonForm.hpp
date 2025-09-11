#pragma once

#include "LogForm.hpp"

class JsonForm : public LogForm {
public:
	virtual std::string format(const LogMessage& msg);

private:
	std::string escapeJson(const std::string& str) const;
};
