#pragma once

#include "LogForm.hpp"
#include <vector>

class ElfForm : public LogForm {
public:
	ElfForm();
	virtual void format(const LogMessage &msg, std::ostream &out);
	virtual void formatAccess(const AccessLogContext& ctx, std::ostream& out);

private:
	void getErrorHeader(std::ostream &out);
	void getAccessHeader(std::ostream &out);

	bool _headerWritten;
};
