#pragma once

#include "../LogType.hpp"
#include "../AccessLog/AccessLogStructure.hpp"
#include "../ErrorLog/LogStructure.hpp"
#include <ostream>
#include <string>

class LogForm {
public:
	virtual ~LogForm();
	virtual void format(const LogMessage &msg, std::ostream &out) = 0;
	virtual void formatAccess(const AccessLogContext& ctx, std::ostream& out) = 0;
	virtual std::string levelToString(LogLevel level) const;
};
