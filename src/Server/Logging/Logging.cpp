#include "Logging.hpp"
#include "../../Lib/Logger/AccessLog/AccessLogger.hpp"
#include "../../Lib/Logger/ErrorLog/Logger.hpp"
#include "../../Lib/Message/Art.hpp"
#include <iostream>
#include <vector>

namespace {

/**
 * @brief アクセスログが無効化されているかをチェックする
 * @return true 無効化されている, false 有効なログ設定が存在する
 */
bool isAccessLoggingDisabled(const std::vector< AccessLog > &logs) {
	for (std::vector< AccessLog >::const_iterator it = logs.begin();
		 it != logs.end(); ++it) {
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
bool isErrorLoggingDisabled(const std::vector< ErrorLog > &logs) {
	for (std::vector< ErrorLog >::const_iterator it = logs.begin();
		 it != logs.end(); ++it) {
		if (it->isDisable) {
			return true;
		}
	}
	return false;
}

/**
 * @brief エラーログでコンソール出力とデバッグレベルの出力が有効かをチェックする
 * @return コンソール出力が有効化されている場合はtrue、そうでなければfalse
 */
bool isConsoleDebugActive(const Config &config) {
	// すべてのserverのエラーログをチェック
	const std::vector< ServerConfig > &servers = config.getServers();
	for (std::vector< ServerConfig >::const_iterator sIt = servers.begin();
		 sIt != servers.end(); ++sIt) {
		for (std::vector< ErrorLog >::const_iterator it =
				 sIt->errorLogs.begin();
			 it != sIt->errorLogs.end(); ++it) {
			if (it->sink == Console && it->level == DEBUG) {
				return true;
			}
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
void setupLoggers(const Config &config) {
	// すべてのserverのログ設定を処理
	const std::vector< ServerConfig > &servers = config.getServers();

	AccessLogger &accessLogger = AccessLogger::getInstance();
	Logger &errorLogger = Logger::getInstance();

	for (std::vector< ServerConfig >::const_iterator sIt = servers.begin();
		 sIt != servers.end(); ++sIt) {
		// AccessLogの設定
		if (!isAccessLoggingDisabled(sIt->accessLogs)) {
			for (std::vector< AccessLog >::const_iterator it =
					 sIt->accessLogs.begin();
				 it != sIt->accessLogs.end(); ++it) {
				if (it->sink == File) {
					accessLogger.setSinkFile(it->logDir, it->filename,
											 it->format, it->maxFileSize,
											 it->maxBackupFiles);
				} else if (it->sink == Console) {
					accessLogger.setSinkConsole(it->format);
				}
			}
		}

		// ErrorLogの設定
		if (!isErrorLoggingDisabled(sIt->errorLogs)) {
			for (std::vector< ErrorLog >::const_iterator it =
					 sIt->errorLogs.begin();
				 it != sIt->errorLogs.end(); ++it) {
				if (it->sink == File) {
					errorLogger.setSinkFile(it->logDir, it->filename, it->format,
											it->level, it->filterMode,
											it->maxFileSize, it->maxBackupFiles);
				} else if (it->sink == Console) {
					errorLogger.setSinkConsole(it->format, it->level,
											   it->filterMode);
				}
			}
		}
	}

	if (isConsoleDebugActive(config)) {
		Art::art();
	}
}
} // namespace Logging
