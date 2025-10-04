#include "Config.hpp"
#include "../Lib/MyYAML/MyYAML.hpp"
#include "../Lib/StringOps/StringOps.hpp"
#include "../Lib/Logger/LogType.hpp"
#include "../Lib/Logger/ErrorLog/Logger.hpp"
#include <iostream>
#include <sstream>
#include <stdexcept>

void Config::setRoot(const std::string &root, const std::string &locationKey) {
	_locations[locationKey].root = root;
}

void Config::setAutoindex(const bool autoindex,
						  const std::string &locationKey) {
	_locations[locationKey].autoindex = autoindex;
}

void Config::setIndexFile(const std::string &indexFile,
						  const std::string &locationKey) {
	_locations[locationKey].indexFile = indexFile;
}

void Config::setErrorFile(const std::string &errorFile,
						  const std::string &locationKey) {
	_locations[locationKey].errorFile = errorFile;
}

void Config::setUploadStore(const std::string &uploadStore,
							const std::string &locationKey) {
	_locations[locationKey].uploadStore = uploadStore;
}

void Config::setCgiConf(const std::string &extension,
						const std::string &interpreterPath,
						const std::string &locationKey) {
	_locations[locationKey].cgiConf[extension] = interpreterPath;
}

void Config::setIsAllowGet(const bool allow, const std::string &locationKey) {
	if (allow)
		_locations[locationKey].allowedMethods.insert("GET");
	else
		_locations[locationKey].allowedMethods.erase("GET");
}

void Config::setIsAllowHead(const bool allow, const std::string &locationKey) {
	if (allow)
		_locations[locationKey].allowedMethods.insert("HEAD");
	else
		_locations[locationKey].allowedMethods.erase("HEAD");
}

void Config::setIsAllowPost(const bool allow, const std::string &locationKey) {
	if (allow)
		_locations[locationKey].allowedMethods.insert("POST");
	else
		_locations[locationKey].allowedMethods.erase("POST");
}

void Config::setIsAllowDelete(const bool allow,
							  const std::string &locationKey) {
	if (allow)
		_locations[locationKey].allowedMethods.insert("DELETE");
	else
		_locations[locationKey].allowedMethods.erase("DELETE");
}

void Config::setAllowedMethods(const std::string &methods,
							   const std::string &locationKey) {
	std::set<std::string> methodSet;
	std::stringstream ss(methods);
	std::string method;
	while (ss >> method) {
		methodSet.insert(method);
	}
	_locations[locationKey].allowedMethods = methodSet;
}

void Config::setAllowedMethods(const std::set<std::string> &methods,
							   const std::string &locationKey) {
	_locations[locationKey].allowedMethods = methods;
}

void Config::setListens(const std::vector<Listen> &lists) { _listens = lists; }

void Config::setRedirects(const std::map<std::string, Redirect> &redirects) {
	_redirects = redirects;
}

void Config::setRedirect(const Redirect &redirect,
						 const std::string &redirectKey) {
	_redirects[redirectKey] = redirect;
}

void Config::setLocations(const std::map<std::string, Location> &locations) {
	_locations = locations;
}

void Config::setLocation(const Location &location,
						 const std::string &locationKey) {
	_locations[locationKey] = location;
}

void Config::setMaxRequestBodySize(const unsigned int size) {
	_maxRequestBodySize = size;
}

void Config::setTimeoutSec(const unsigned int sec) { _timeoutSec = sec; }

void Config::setMaxEvents(const unsigned int maxEvents) {
	_maxEvents = maxEvents;
}

void Config::initDefaults() {
	_listens.clear();
	_redirects.clear();
	_locations.clear();
	_accessLogs.clear();
	_errorLogs.clear();

	_maxRequestBodySize = 1024 * 1024;
	_timeoutSec = 60;
	_maxEvents = 1024;

	Location defaultLoc;
	defaultLoc.path = "/";
	defaultLoc.root = "/tmp/www";
	defaultLoc.uploadStore = "/tmp/uploads";
	defaultLoc.indexFile = "index.html";
	defaultLoc.autoindex = true;
	defaultLoc.allowedMethods.insert("GET");
	defaultLoc.allowedMethods.insert("HEAD");
	defaultLoc.allowedMethods.insert("POST");
	defaultLoc.allowedMethods.insert("DELETE");
	_locations["/"] = defaultLoc;

	_accessLogs.push_back(AccessLog());
	_errorLogs.push_back(ErrorLog());
}

Config::Config() {
	initDefaults();
	setup("config/default.yaml");
}

Config::Config(const std::string &configFile) {
	initDefaults();
	setup(configFile);
}

Config::Config(const Config &other)
	: _listens(other._listens), _redirects(other._redirects),
	  _locations(other._locations), _accessLogs(other._accessLogs),
	  _errorLogs(other._errorLogs),
	  _maxRequestBodySize(other._maxRequestBodySize),
	  _timeoutSec(other._timeoutSec), _maxEvents(other._maxEvents) {}

Config &Config::operator=(const Config &other) {
	if (this != &other) {
		_listens = other._listens;
		_redirects = other._redirects;
		_locations = other._locations;
		_accessLogs = other._accessLogs;
		_errorLogs = other._errorLogs;
		_maxRequestBodySize = other._maxRequestBodySize;
		_timeoutSec = other._timeoutSec;
		_maxEvents = other._maxEvents;
	}
	return *this;
}

Config::~Config() {}

void Config::parseListens(const Node *node) {
	if (!node)
		throw std::runtime_error("Config error: missing 'listens' node");
	const std::vector<Node *> &listens = node->getSeq();
	for (std::vector<Node *>::const_iterator it = listens.begin();
		 it != listens.end(); ++it) {
		Node *l_node = *it;
		if (l_node->getKey() != "listen") {
			throw std::runtime_error(
				"Config error: missing 'listen' key in listen item");
		}

		Listen l;
		Node *interfaceNode = l_node->getMapNode("interface");
		if (!interfaceNode)
			throw std::runtime_error(
				"Config error: missing 'interface' in listen item");
		l.interface = interfaceNode->getValue();

		Node *portNode = l_node->getMapNode("port");
		if (!portNode)
			throw std::runtime_error(
				"Config error: missing 'port' in listen item");
		int port = StringOps::stringToInt(portNode->getValue());
		if (port < 1024 || port > 65535) {
			std::stringstream ss;
			ss << "Config error: invalid port number " << port
			   << ". Port must be between 1024 and 65535.";
			throw std::runtime_error(ss.str());
		}
		l.port = port;

		_listens.push_back(l);
	}
}

void Config::parseRedirects(Node *node) {
	if (!node)
		throw std::runtime_error("Config error: missing 'redirects' node");
	const std::vector<Node *> &redirects = node->getSeq();
	for (std::vector<Node *>::const_iterator it = redirects.begin();
		 it != redirects.end(); ++it) {
		Node *r_node = *it;
		if (r_node->getKey() != "redirect") {
			continue;
		}

		Redirect r;
		Node *fromNode = r_node->getMapNode("from");
		if (!fromNode)
			throw std::runtime_error(
				"Config error: missing 'from' key in redirect item");
		r.fromPath = fromNode->getValue();

		Node *toNode = r_node->getMapNode("to");
		if (!toNode)
			throw std::runtime_error(
				"Config error: missing 'to' key in redirect item");
		r.toUrl = toNode->getValue();

		Node *codeNode = r_node->getMapNode("code");
		if (!codeNode)
			throw std::runtime_error(
				"Config error: missing 'code' key in redirect item");
		r.code = StringOps::stringToInt(codeNode->getValue());

		_redirects[r.fromPath] = r;
	}
}

void Config::parseLocations(Node *node) {
	if (!node)
		throw std::runtime_error("Config error: missing 'locations' node");

	const char *validMethodsArr[] = {"GET", "POST", "HEAD", "DELETE"};
	std::set<std::string> validMethods(validMethodsArr, validMethodsArr + 4);

	const std::vector<Node *> &locations = node->getSeq();
	for (std::vector<Node *>::const_iterator it = locations.begin();
		 it != locations.end(); ++it) {
		Node *l_node = *it;
		if (l_node->getKey() != "location") {
			continue;
		}

		Location loc;
		Node *pathNode = l_node->getMapNode("path");
		if (!pathNode)
			throw std::runtime_error(
				"Config error: missing 'path' key in location item");
		loc.path = pathNode->getValue();

		Node *rootNode = l_node->getMapNode("root");
		if (rootNode)
			loc.root = rootNode->getValue();
		Node *errorFileNode = l_node->getMapNode("errorFile");
		if (errorFileNode)
			loc.errorFile = errorFileNode->getValue();
		Node *uploadStoreNode = l_node->getMapNode("uploadStore");
		if (uploadStoreNode)
			loc.uploadStore = uploadStoreNode->getValue();
		Node *indexNode = l_node->getMapNode("indexFile");
		if (indexNode)
			loc.indexFile = indexNode->getValue();
		Node *autoindexNode = l_node->getMapNode("autoindex");
		if (autoindexNode)
			loc.autoindex = (autoindexNode->getValue() == "true");

		if (Node *allowMethodsNode = l_node->getMapNode("allowedMethods")) {
			const std::vector<Node *> &methods = allowMethodsNode->getSeq();
			for (std::vector<Node *>::const_iterator m_it = methods.begin();
				 m_it != methods.end(); ++m_it) {
				std::string method = (*m_it)->getValue();
				if (validMethods.find(method) == validMethods.end()) {
					throw std::runtime_error(
						"Config error: invalid HTTP method '" + method + "'");
				}
				loc.allowedMethods.insert(method);
			}
		}

		if (Node *cgiNode = l_node->getMapNode("cgi")) {
			const std::vector<std::string> extensions = cgiNode->getKeys();
			for (std::vector<std::string>::const_iterator ext_it = extensions.begin();
				 ext_it != extensions.end(); ++ext_it) {
				const std::string& extension = *ext_it;
				Node *interpreterNode = cgiNode->getMapNode(extension);
				const std::string& interpreterPath = interpreterNode->getValue();

				if (extension.empty() || extension[0] != '.') {
					throw std::runtime_error("Config error: CGI extension must start with a '.'. Found: " + extension);
				}
				if (interpreterPath.empty()) {
					throw std::runtime_error("Config error: CGI interpreter path cannot be empty for extension " + extension);
				}

				loc.cgiConf[extension] = interpreterPath;
			}
		}

		_locations[loc.path] = loc;
	}
}

void Config::parseAccessLogs(Node *node) {
	if (!node) return;

	std::vector<AccessLog> configuredLogs;
	const std::vector<Node *> &logs = node->getSeq();

	bool isDisabledFound = false;
	bool isEnabledFound = false;

	for (std::vector<Node *>::const_iterator it = logs.begin(); it != logs.end(); ++it) {
		Node *logNode = *it;
		if (logNode->getKey() != "access_log") {
			continue;
		}

		AccessLog log;

		Node *disableNode = logNode->getMapNode("disable");
		Node *sinkNode = logNode->getMapNode("sink");
		Node *filenameNode = logNode->getMapNode("filename");
		Node *logDirNode = logNode->getMapNode("logDir");
		Node *formatNode = logNode->getMapNode("format");
		Node *maxSizeNode = logNode->getMapNode("maxSize");
		Node *maxBackupNode = logNode->getMapNode("maxBackup");

		const bool isDisabled = disableNode && StringOps::equalsIgnoreCase(
											 disableNode->getValue(), "true");

		if (isDisabled) {
			isDisabledFound = true;
		} else {
			isEnabledFound = true;
		}

		if (isDisabled) {
			if (sinkNode || filenameNode || logDirNode || formatNode || maxSizeNode || maxBackupNode) {
				throw std::runtime_error("Config error in access_log: When 'disable' is true, other directives are not allowed.");
			}
			log.isDisable = true;
			configuredLogs.push_back(log);
			continue;
		}

		log.isDisable = false;

		if (!sinkNode) throw std::runtime_error("Config error in access_log: 'sink' is required.");
		const std::string sinkValue = StringOps::toUpper(sinkNode->getValue());
		if (sinkValue == "FILE") log.sink = File;
		else if (sinkValue == "CONSOLE") log.sink = Console;
		else throw std::runtime_error("Config error in access_log: 'sink' must be 'file' or 'console'.");

		if (formatNode) {
			const std::string formatValue = StringOps::toUpper(formatNode->getValue());
			if (formatValue == "JSON") log.format = JSON;
			else if (formatValue == "ELF") log.format = ELF;
			else throw std::runtime_error("Config error in access_log: invalid format type.");
		}

		if (sinkValue == "FILE") {
			if (filenameNode)
				log.filename = filenameNode->getValue();
			if (logDirNode)
				log.logDir = logDirNode->getValue();

			if (maxSizeNode) log.maxFileSize = StringOps::sizeByteStrToSizeT(maxSizeNode->getValue());
			if (maxBackupNode) log.maxBackupFiles = StringOps::toSizeT(maxBackupNode->getValue());
		} else {
			if (filenameNode) throw std::runtime_error("Config error in access_log: 'filename' is not allowed for 'console' sink.");
			if (logDirNode) throw std::runtime_error("Config error in access_log: 'logDir' is not allowed for 'console' sink.");
			if (maxSizeNode) throw std::runtime_error("Config error in access_log: 'maxSize' is not allowed for 'console' sink.");
			if (maxBackupNode) throw std::runtime_error("Config error in access_log: 'maxBackup' is not allowed for 'console' sink.");
		}
		configuredLogs.push_back(log);
	}
	if (isDisabledFound && isEnabledFound) {
		throw std::runtime_error("Config error in access_log: Cannot mix 'disable: true' with other valid access log configurations.");
	}
	if (!configuredLogs.empty()) {
		_accessLogs = configuredLogs;
	}
}

void Config::parseErrorLogs(Node *node) {
	if (!node) return;

	std::vector<ErrorLog> configuredLogs;
	const std::vector<Node *> &logs = node->getSeq();

	bool isDisabledFound = false;
	bool isEnabledFound = false;

	for (std::vector<Node *>::const_iterator it = logs.begin(); it != logs.end(); ++it) {
		Node *logNode = *it;
		if (logNode->getKey() != "error_log") {
			continue;
		}

		ErrorLog log;

		Node *disableNode = logNode->getMapNode("disable");
		Node *sinkNode = logNode->getMapNode("sink");
		Node *filenameNode = logNode->getMapNode("filename");
		Node *logDirNode = logNode->getMapNode("logDir");
		Node *formatNode = logNode->getMapNode("format");
		Node *levelNode = logNode->getMapNode("level");
		Node *modeNode = logNode->getMapNode("mode");
		Node *maxSizeNode = logNode->getMapNode("maxSize");
		Node *maxBackupNode = logNode->getMapNode("maxBackup");

		const bool isDisabled = disableNode && StringOps::equalsIgnoreCase(
											 disableNode->getValue(), "true");

		if (isDisabled) {
			isDisabledFound = true;
		} else {
			isEnabledFound = true;
		}

		if (isDisabled) {
			if (sinkNode || filenameNode || logDirNode || formatNode || levelNode || modeNode || maxSizeNode || maxBackupNode) {
				throw std::runtime_error("Config error in error_log: When 'disable' is true, other directives are not allowed.");
			}
			log.isDisable = true;
			configuredLogs.push_back(log);
			continue;
		}

		log.isDisable = false;

		if (!sinkNode) throw std::runtime_error("Config error in error_log: 'sink' is required.");
		const std::string sinkValue = StringOps::toUpper(sinkNode->getValue());
		if (sinkValue == "FILE") log.sink = File;
		else if (sinkValue == "CONSOLE") log.sink = Console;
		else throw std::runtime_error("Config error in error_log: 'sink' must be 'file' or 'console'.");

		if (formatNode) {
			const std::string formatValue = StringOps::toUpper(formatNode->getValue());
			if (formatValue == "JSON") log.format = JSON;
			else if (formatValue == "ELF") log.format = ELF;
			else throw std::runtime_error("Config error in error_log: invalid format type.");
		}

		if (levelNode) {
			const std::string levelValue = StringOps::toUpper(levelNode->getValue());
			if (levelValue == "DEBUG") log.level = DEBUG;
			else if (levelValue == "INFO") log.level = INFO;
			else if (levelValue == "WARNING") log.level = WARNING;
			else if (levelValue == "ERROR") log.level = ERROR;
			else if (levelValue == "FATAL") log.level = FATAL;
			else throw std::runtime_error("Config error in error_log: invalid log level.");
		}

		if (modeNode) {
			const std::string modeValue = StringOps::toUpper(modeNode->getValue());
			if (modeValue == "GREATER_OR_EQUAL") log.filterMode = GREATER_OR_EQUAL;
			else if (modeValue == "EXACT") log.filterMode = EXACT;
			else throw std::runtime_error("Config error in error_log: invalid filter mode.");
		}

		if (sinkValue == "FILE") {
			if (filenameNode)
				log.filename = filenameNode->getValue();
			if (logDirNode)
				log.logDir = logDirNode->getValue();

			if (maxSizeNode) log.maxFileSize = StringOps::sizeByteStrToSizeT(maxSizeNode->getValue());
			if (maxBackupNode) log.maxBackupFiles = StringOps::toSizeT(maxBackupNode->getValue());
		} else {
			if (filenameNode) throw std::runtime_error("Config error in error_log: 'filename' is not allowed for 'console' sink.");
			if (logDirNode) throw std::runtime_error("Config error in error_log: 'logDir' is not allowed for 'console' sink.");
			if (maxSizeNode) throw std::runtime_error("Config error in error_log: 'maxSize' is not allowed for 'console' sink.");
			if (maxBackupNode) throw std::runtime_error("Config error in error_log: 'maxBackup' is not allowed for 'console' sink.");
		}
		configuredLogs.push_back(log);
	}
	if (isDisabledFound && isEnabledFound) {
		throw std::runtime_error("Config error in error_log: Cannot mix 'disable: true' with other valid error log configurations.");
	}
	if (!configuredLogs.empty()) {
		_errorLogs = configuredLogs;
	}
}

void Config::setup(const std::string &configFile) {
	LOG(INFO) << "Loading configuration from: " << configFile;
	const MyYAML yaml(configFile);
	const Node *serversNode = yaml.getData().getMapNode("servers");
	if (!serversNode) {
		throw std::runtime_error("Config error: missing 'servers' root node");
	}

	const std::vector<Node *> &serverList = serversNode->getSeq();
	if (serverList.empty()) {
		throw std::runtime_error("Config error: no servers configured");
	}

	Node *serverNode = serverList[0];
	if (serverNode->getKey() != "server") {
		throw std::runtime_error(
			"Config error: missing 'server' key in server list");
	}

	parseListens(serverNode->getMapNode("listens"));
	if (Node *redirectsNode = serverNode->getMapNode("redirects")) {
		parseRedirects(redirectsNode);
	}
	if (Node *locationsNode = serverNode->getMapNode("locations")) {
		parseLocations(locationsNode);
	}
	if (Node *accessLogsNode = serverNode->getMapNode("access_logs")) {
		parseAccessLogs(accessLogsNode);
	}
	if (Node *errorLogsNode = serverNode->getMapNode("error_logs")) {
		parseErrorLogs(errorLogsNode);
	}

	if (Node *n = serverNode->getMapNode("maxRequestBodySize"))
		_maxRequestBodySize = StringOps::stringToInt(n->getValue());

	if (Node *n = serverNode->getMapNode("timeoutSec"))
		_timeoutSec = StringOps::stringToInt(n->getValue());

	if (Node *n = serverNode->getMapNode("maxEvents"))
		_maxEvents = StringOps::stringToInt(n->getValue());
}

const std::vector<Listen> &Config::getListens() const { return _listens; }

const std::map<std::string, Redirect> &Config::getRedirects() const {
	return _redirects;
}

const Redirect &Config::getRedirect(const std::string &path) const {
    std::string bestMatchKey = "";

    for (std::map<std::string, Redirect>::const_iterator it = _redirects.begin();
         it != _redirects.end(); ++it) {
        const std::string &redirectPath = it->first;
        if (path.rfind(redirectPath, 0) == 0) {
            if (redirectPath.length() > bestMatchKey.length()) {
                bestMatchKey = redirectPath;
            }
        }
    }
    if (!bestMatchKey.empty()) {
        return _redirects.at(bestMatchKey);
    }
    // Return a default constructed Redirect indicating no match
    static const Redirect noMatchRedirect = {"", "", 0};
    return noMatchRedirect;
}

const std::map<std::string, Location> &Config::getLocations() const {
	return _locations;
}

const Location &Config::getLocation(const std::string &path) const {
	std::string bestMatchKey = "";

	for (std::map<std::string, Location>::const_iterator it =
			 _locations.begin();
		 it != _locations.end(); ++it) {
		const std::string &locPath = it->first;
		if (path.rfind(locPath, 0) == 0) {
			if (locPath.length() > bestMatchKey.length()) {
				bestMatchKey = locPath;
			}
		}
	}
	if (!bestMatchKey.empty()) {
		std::map<std::string, Location>::const_iterator it =
			_locations.find(bestMatchKey);
		return it->second;
	}
	std::map<std::string, Location>::const_iterator it = _locations.find("/");
	if (it != _locations.end()) {
		return it->second;
	}
	throw std::runtime_error("Config error: default location '/' not found");
}

const std::vector<AccessLog> &Config::getAccessLogs() const {
	return _accessLogs;
}

const std::vector<ErrorLog> &Config::getErrorLogs() const {
	return _errorLogs;
}

unsigned int Config::getMaxRequestBodySize() const {
	return _maxRequestBodySize;
}
unsigned int Config::getTimeoutSec() const { return _timeoutSec; }
unsigned int Config::getMaxEvents() const { return _maxEvents; }

std::ostream &operator<<(std::ostream &os, const Config &config) {
	os << "Config:\n";
	os << "  maxRequestBodySize: " << config._maxRequestBodySize << "\n";
	os << "  timeoutSec: " << config._timeoutSec << "\n";
	os << "  maxEvents: " << config._maxEvents << "\n";

	os << "  listens:\n";
	for (std::vector<Listen>::const_iterator it = config._listens.begin();
		 it != config._listens.end(); ++it) {
		os << "    - " << it->interface << ":" << it->port << "\n";
	}

	os << "  redirects:\n";
	for (std::map<std::string, Redirect>::const_iterator it =
			 config._redirects.begin();
		 it != config._redirects.end(); ++it) {
		os << "    - from: " << it->second.fromPath
		   << ", to: " << it->second.toUrl << ", code: " << it->second.code
		   << "\n";
	}

	os << "  locations:\n";
	for (std::map<std::string, Location>::const_iterator it =
			 config._locations.begin();
		 it != config._locations.end(); ++it) {
		os << "  - path: " << it->second.path << "\n";
		os << "      root: " << it->second.root << "\n";
		os << "      allowedMethods: ";
		for (std::set<std::string>::const_iterator mit =
				 it->second.allowedMethods.begin();
			 mit != it->second.allowedMethods.end(); ++mit) {
			os << *mit << " ";
		}
		os << "\n";
		os << "      autoindex: " << (it->second.autoindex ? "on" : "off")
		   << "\n";
		os << "      indexFile: " << it->second.indexFile << "\n";
		os << "      errorFile: " << it->second.errorFile << "\n";
		os << "      uploadStore: " << it->second.uploadStore << "\n";
		os << "      cgiConf:\n";
		for (std::map<std::string, std::string>::const_iterator cit =
				 it->second.cgiConf.begin();
			 cit != it->second.cgiConf.end(); ++cit) {
			os << "        " << cit->first << ": " << cit->second << "\n";
		}
	}

	os << "  accessLogs:\n";
	for (std::vector<AccessLog>::const_iterator it = config._accessLogs.begin();
		 it != config._accessLogs.end(); ++it) {
		os << "    - isDisable: " << (it->isDisable ? "true" : "false") << "\n";
		os << "      sink: " << (it->sink == File ? "file" : "console") << "\n";
		os << "      filename: " << it->filename << "\n";
		os << "      logDir: " << it->logDir << "\n";
		os << "      format: " << (it->format == JSON ? "json" : "elf") << "\n";
		os << "      maxFileSize: " << it->maxFileSize << "\n";
		os << "      maxBackupFiles: " << it->maxBackupFiles << "\n";
	}

	os << "  errorLogs:\n";
	for (std::vector<ErrorLog>::const_iterator it = config._errorLogs.begin();
		 it != config._errorLogs.end(); ++it) {
		os << "    - isDisable: " << (it->isDisable ? "true" : "false") << "\n";
		os << "      sink: " << (it->sink == File ? "FILE" : "CONSOLE") << "\n";
		os << "      filename: " << it->filename << "\n";
		os << "      logDir: " << it->logDir << "\n";
		os << "      format: " << (it->format == JSON ? "JSON" : "ELF") << "\n";
		os << "      level: ";
		switch (it->level) {
		case DEBUG:
			os << "DEBUG";
			break;
		case INFO:
			os << "INFO";
			break;
		case WARNING:
			os << "WARNING";
			break;
		case ERROR:
			os << "ERROR";
			break;
		case FATAL:
			os << "FATAL";
			break;
		}
		os << "\n";
		os << "      filterMode: "
		   << (it->filterMode == GREATER_OR_EQUAL ? "GREATER_OR_EQUAL"
												  : "EXACT")
		   << "\n";
		os << "      maxFileSize: " << it->maxFileSize << "\n";
		os << "      maxBackupFiles: " << it->maxBackupFiles << "\n";
	}
	return os;
}
