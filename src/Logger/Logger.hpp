#pragma once

#include <string>
#include <fstream>
#include <sstream>

enum LogLevel {
	DEBUG,
	INFO,
	WARNING,
	ERROR,
	FATAL
};

class Logger {
public:
	static Logger& getInstance();

	// ログを出力するメイン関数
	void log(LogLevel level, const std::string& message);

	// ログレベルと出力ファイルを設定
	void setLogLevel(LogLevel level);
	void setOutputFile(const std::string& filename);

private:
	Logger();
	Logger(const Logger&);
	Logger& operator=(const Logger&);
	~Logger();

	static Logger* _instance;
	LogLevel _logLevel;
	std::ofstream _logFile;

	std::string levelToString(LogLevel level);
};

#define LOG_DEBUG(msg) do { std::stringstream ss; ss << msg; Logger::getInstance().log(DEBUG, ss.str()); } while(0)
#define LOG_INFO(msg) do { std::stringstream ss; ss << msg; Logger::getInstance().log(INFO, ss.str()); } while(0)
#define LOG_WARNING(msg) do { std::stringstream ss; ss << msg; Logger::getInstance().log(WARNING, ss.str()); } while(0)
#define LOG_ERROR(msg) do { std::stringstream ss; ss << msg; Logger::getInstance().log(ERROR, ss.str()); } while(0)
#define LOG_FATAL(msg) do { std::stringstream ss; ss << msg; Logger::getInstance().log(FATAL, ss.str()); } while(0)
