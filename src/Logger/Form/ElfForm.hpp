#pragma once

#include "LogForm.hpp"
#include <vector>

class ElfForm : public LogForm {
public:
	ElfForm();
	virtual std::string format(const LogMessage& msg);

private:
	std::string getHeader();
	std::string sanitize(const std::string& str) const;

	bool _headerWritten;
};
