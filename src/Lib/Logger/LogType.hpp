#pragma once

#define _LOG_FALLBACK_DIR "./"
#define _LOG_DEFAULT_DIR "./logs/"

enum LogSinkType {
	File,
	Console
};

enum LogType {
	errorLog,
	accessLog
};

enum LogFormat { JSON, ELF };
