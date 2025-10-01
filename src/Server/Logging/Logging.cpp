#include "Logging.hpp"
#include "../../Lib/Logger/AccessLog/AccessLogger.hpp"
#include "../../Lib/Logger/ErrorLog/Logger.hpp"
#include <vector>
#include <iostream>

namespace {

/**
 * @brief アクセスログが無効化されているかをチェックする
 * @return true 無効化されている, false 有効なログ設定が存在する
 */
bool isAccessLoggingDisabled(const std::vector<AccessLog>& logs) {
	for (std::vector<AccessLog>::const_iterator it = logs.begin(); it != logs.end(); ++it) {
		if (it->isDisable) {
			return true;
		}
	}
	return false;
}

/**
 * @brief エラーログが無効化されているかをチェックする
 * @return 無効化されている場合はtrue、そうでなければfalse
 */
bool isErrorLoggingDisabled(const std::vector<ErrorLog>& logs) {
	for (std::vector<ErrorLog>::const_iterator it = logs.begin(); it != logs.end(); ++it) {
		if (it->isDisable) {
			return true;
		}
	}
	return false;
}
} // namespace

namespace Logging {

/**
 * @brief ロガーを設定する
 * @param config 設定情報
 */
void setupLoggers(const Config& config) {
	std::cerr << "[42/Webserv] Setting up loggers..." << std::endl;
	const std::vector<AccessLog>& accessLogs = config.getAccessLogs();
	std::cerr << "[42/Webserv] Access log entries: " << accessLogs.size() << std::endl;
	if (!isAccessLoggingDisabled(accessLogs)) {

		AccessLogger& accessLogger = AccessLogger::getInstance();

		std::vector<AccessLog>::const_iterator it;
		for (it = accessLogs.begin(); it != accessLogs.end(); ++it) {
			if (it->sink == File) {
				std::cerr << "[42/Webserv AccessLog] " << it->filename << " opened.";
				accessLogger.setSinkFile(it->logDir, it->filename, it->format, it->maxFileSize, it->maxBackupFiles);
			} else if (it->sink == Console) {
				accessLogger.setSinkConsole(it->format);
			}
		}
	}

	const std::vector<ErrorLog>& errorLogs = config.getErrorLogs();
	if (!isErrorLoggingDisabled(errorLogs)) {

		Logger& errorLogger = Logger::getInstance();

		std::vector<ErrorLog>::const_iterator it;
		for (it = errorLogs.begin(); it != errorLogs.end(); ++it) {
			if (it->sink == File) {
				std::cerr << "[42/Webserv ErrorLog] " << it->filename << " opened.";
				errorLogger.setSinkFile(it->logDir, it->filename, it->format, it->level, it->filterMode, it->maxFileSize, it->maxBackupFiles);
			} else if (it->sink == Console) {
				errorLogger.setSinkConsole(it->format, it->level, it->filterMode);
			}
		}
	}
}
} // namespace Logging
