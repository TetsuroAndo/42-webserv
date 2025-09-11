#pragma once

#include "../LogStructure.hpp"
#include <string>

class LogForm {
public:
	virtual ~LogForm();
	virtual std::string format(const LogMessage& msg) = 0;
	virtual std::string levelToString(LogLevel level) const;
};
