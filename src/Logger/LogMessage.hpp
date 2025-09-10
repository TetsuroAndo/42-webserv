#pragma once

class LogMessage {
public:
	LogMessage(const std::string& message);
	const std::string& getMessage() const;

private:
	std::string _message;
};
