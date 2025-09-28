#pragma once

#include "LogForm.hpp"
#include <vector>

class ElfForm : public LogForm {
public:
	ElfForm();
	virtual void format(const LogMessage &msg, std::ostream &out);
	virtual void formatAccess(const AccessLogContext& ctx, std::ostream& out);

private:
	std::string getHeader();
	std::string sanitize(const std::string &str) const;

	bool _headerWritten;
};
