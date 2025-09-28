#pragma once

#include "LogForm.hpp"

class AccessLogForm : public LogForm {
public:
	virtual void format(const LogMessage& msg, std::ostream& out);
};
