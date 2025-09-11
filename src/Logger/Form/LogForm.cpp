#include "LogForm.hpp"

LogForm::~LogForm() {}

/**
 * @brief log levelを文字列に変換
 * @param level ログレベル
 * @return ログレベルに対応する文字列
 */
std::string LogForm::levelToString(LogLevel level) const {
	switch (level) {
		case DEBUG: return "DEBUG";
		case INFO: return "INFO";
		case WARNING: return "WARNING";
		case ERROR: return "ERROR";
		case FATAL: return "FATAL";
		default: return "UNKNOWN";
	}
}
