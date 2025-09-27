#pragma once

#include "LogForm.hpp"

class JsonForm : public LogForm {
public:
	virtual void format(const LogMessage &msg, std::ostream &out);

private:
	std::string escapeJson(const std::string &str) const;
};
