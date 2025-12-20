#include "ConfigLogParser.hpp"
#include "../Lib/Logger/LogType.hpp"
#include "../Lib/MyYAML/MyYAML.hpp"
#include "../Lib/StringOps/StringOps.hpp"
#include "Config.hpp"
#include "ConfigBuilder.hpp"
#include "ConfigParser.hpp"
#include <set>
#include <sstream>
#include <stdexcept>
#include <vector>

ConfigLogParser::ConfigLogParser(ConfigBuilder *builder) : _builder(builder) {}
ConfigLogParser::~ConfigLogParser() {}

namespace {
void parseLogSpecifics(Node *logNode, AccessLog &log,
					   const std::string &logKey) {
	(void)logNode;
	(void)log;
	(void)logKey;
}

void parseLogSpecifics(Node *logNode, ErrorLog &log,
					   const std::string &logKey) {
	Node *levelNode = logNode->getMapNode("level");
	if (levelNode) {
		const std::string levelValue =
			StringOps::toUpper(levelNode->getValue());
		if (levelValue == "DEBUG")
			log.level = DEBUG;
		else if (levelValue == "INFO")
			log.level = INFO;
		else if (levelValue == "WARNING")
			log.level = WARNING;
		else if (levelValue == "ERROR")
			log.level = ERROR;
		else if (levelValue == "FATAL")
			log.level = FATAL;
		else
			throw std::runtime_error("Config error in " + logKey +
									 ": invalid log level.");
	}

	Node *modeNode = logNode->getMapNode("mode");
	if (modeNode) {
		const std::string modeValue = StringOps::toUpper(modeNode->getValue());
		if (modeValue == "GREATER_OR_EQUAL")
			log.filterMode = GREATER_OR_EQUAL;
		else if (modeValue == "EXACT")
			log.filterMode = EXACT;
		else
			throw std::runtime_error("Config error in " + logKey +
									 ": invalid filter mode.");
	}
}

template < typename LogType >
void parseLogs(const Node *node, const std::string &logKey,
			   const std::set< std::string > &enabledValidKeys,
			   std::vector< LogType > &configuredLogs) {
	if (!node)
		return;

	const std::vector< Node * > &logs = node->getSeq();
	bool isDisabledFound = false;
	bool isEnabledFound = false;

	for (std::vector< Node * >::const_iterator it = logs.begin();
		 it != logs.end(); ++it) {
		Node *logNode = *it;
		if (logNode->getKey() != logKey) {
			throw std::runtime_error("Configuration error: invalid key '" +
									 logNode->getKey() + "'; expected '" +
									 logKey + "'.");
		}

		LogType log;

		Node *disableNode = logNode->getMapNode("disable");
		const bool isDisabled =
			disableNode &&
			StringOps::equalsIgnoreCase(disableNode->getValue(), "true");

		if (isDisabled) {
			isDisabledFound = true;
			ConfigParser::validateKeys(
				logNode,
				(logKey == "access_log"
					 ? ConfigParser::VALID_DISABLED_ACCESS_LOG_KEYS
					 : ConfigParser::VALID_DISABLED_ERROR_LOG_KEYS),
				"disabled " + logKey + " block");
			log.isDisable = true;
			configuredLogs.push_back(log);
			continue;
		}

		isEnabledFound = true;
		ConfigParser::validateKeys(logNode, enabledValidKeys,
								   logKey + " block");

		log.isDisable = false;

		Node *sinkNode = logNode->getMapNode("sink");
		if (!sinkNode)
			throw std::runtime_error("Config error in " + logKey +
									 ": 'sink' is required.");
		const std::string sinkValue = StringOps::toUpper(sinkNode->getValue());
		if (sinkValue == "FILE")
			log.sink = File;
		else if (sinkValue == "CONSOLE")
			log.sink = Console;
		else
			throw std::runtime_error("Config error in " + logKey +
									 ": 'sink' must be 'file' or 'console'.");

		Node *formatNode = logNode->getMapNode("format");
		if (formatNode) {
			const std::string formatValue =
				StringOps::toUpper(formatNode->getValue());
			if (formatValue == "JSON")
				log.format = JSON;
			else if (formatValue == "ELF")
				log.format = ELF;
			else
				throw std::runtime_error("Config error in " + logKey +
										 ": invalid format type.");
		}

		if (sinkValue == "FILE") {
			Node *filenameNode = logNode->getMapNode("filename");
			if (filenameNode)
				log.filename = filenameNode->getValue();
			Node *logDirNode = logNode->getMapNode("logDir");
			if (logDirNode)
				log.logDir = logDirNode->getValue();
			Node *maxSizeNode = logNode->getMapNode("maxSize");
			if (maxSizeNode)
				log.maxFileSize =
					StringOps::sizeByteStrToSizeT(maxSizeNode->getValue());
			Node *maxBackupNode = logNode->getMapNode("maxBackup");
			if (maxBackupNode)
				log.maxBackupFiles =
					StringOps::toSizeT(maxBackupNode->getValue());
		} else {
			if (logNode->getMapNode("filename"))
				throw std::runtime_error(
					"Config error in " + logKey +
					": 'filename' is not allowed for 'console' sink.");
			if (logNode->getMapNode("logDir"))
				throw std::runtime_error(
					"Config error in " + logKey +
					": 'logDir' is not allowed for 'console' sink.");
			if (logNode->getMapNode("maxSize"))
				throw std::runtime_error(
					"Config error in " + logKey +
					": 'maxSize' is not allowed for 'console' sink.");
			if (logNode->getMapNode("maxBackup"))
				throw std::runtime_error(
					"Config error in " + logKey +
					": 'maxBackup' is not allowed for 'console' sink.");
		}

		parseLogSpecifics(logNode, log, logKey);

		configuredLogs.push_back(log);
	}

	if (isDisabledFound && isEnabledFound) {
		throw std::runtime_error("Config error in " + logKey +
								 ": Cannot mix 'disable: true' with other "
								 "valid log configurations.");
	}
}
} // namespace

void ConfigLogParser::parseAccessLogs(const Node *node) {
	std::vector< AccessLog > configuredLogs;
	parseLogs(node, "access_log", ConfigParser::VALID_ACCESS_LOG_KEYS,
			  configuredLogs);

	if (!configuredLogs.empty()) {
		_builder->setAccessLogs(configuredLogs);
	}
}

void ConfigLogParser::parseErrorLogs(const Node *node) {
	std::vector< ErrorLog > configuredLogs;
	parseLogs(node, "error_log", ConfigParser::VALID_ERROR_LOG_KEYS,
			  configuredLogs);

	if (!configuredLogs.empty()) {
		_builder->setErrorLogs(configuredLogs);
	}
}
