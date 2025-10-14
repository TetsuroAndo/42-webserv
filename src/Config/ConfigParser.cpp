#include "ConfigParser.hpp"
#include "../Lib/MyYAML/MyYAML.hpp"
#include "../Lib/StringOps/StringOps.hpp"
#include "Config.hpp"
#include "ConfigLocationParser.hpp"
#include "ConfigLogParser.hpp"
#include <set>
#include <sstream>
#include <stdexcept>
#include <vector>

ConfigParser::ConfigParser(Config *config) : _config(config) {}
ConfigParser::~ConfigParser() {}

void ConfigParser::validateKeys(const Node *node,
								const std::set<std::string> &validKeys,
								const std::string &context) {
	if (!node)
		return;
	const std::vector<std::string> &keys = node->getKeys();
	for (std::vector<std::string>::const_iterator it = keys.begin();
		 it != keys.end(); ++it) {
		if (validKeys.find(*it) == validKeys.end()) {
			throw std::runtime_error("Config error: unknown directive '" + *it +
									 "' in " + context);
		}
	}
}

static std::set<std::string> createValidServerKeys() {
	std::set<std::string> keys;
	keys.insert("listens");
	keys.insert("redirects");
	keys.insert("locations");
	keys.insert("access_logs");
	keys.insert("error_logs");
	keys.insert("maxRequestBodySize");
	keys.insert("timeoutSec");
	keys.insert("maxEvents");
	keys.insert("path");
	keys.insert("root");
	keys.insert("allowedMethods");
	keys.insert("autoindex");
	keys.insert("indexFile");
	keys.insert("errorFile");
	keys.insert("uploadStore");
	keys.insert("cgi");
	return keys;
}

static std::set<std::string> createValidListenKeys() {
	std::set<std::string> keys;
	keys.insert("interface");
	keys.insert("port");
	return keys;
}

static std::set<std::string> createValidRedirectKeys() {
	std::set<std::string> keys;
	keys.insert("from");
	keys.insert("to");
	keys.insert("code");
	return keys;
}

static std::set<std::string> createValidLocationKeys() {
	std::set<std::string> keys;
	keys.insert("path");
	keys.insert("root");
	keys.insert("allowedMethods");
	keys.insert("autoindex");
	keys.insert("indexFile");
	keys.insert("uploadStore");
	keys.insert("cgi");
	keys.insert("redirects");
	keys.insert("errorFile");
	return keys;
}

static std::set<std::string> createValidAccessLogKeys() {
	std::set<std::string> keys;
	keys.insert("disable");
	keys.insert("sink");
	keys.insert("filename");
	keys.insert("logDir");
	keys.insert("format");
	keys.insert("maxSize");
	keys.insert("maxBackup");
	return keys;
}

static std::set<std::string> createValidErrorLogKeys() {
	std::set<std::string> keys;
	keys.insert("disable");
	keys.insert("sink");
	keys.insert("filename");
	keys.insert("logDir");
	keys.insert("format");
	keys.insert("level");
	keys.insert("mode");
	keys.insert("maxSize");
	keys.insert("maxBackup");
	return keys;
}

static std::set<std::string> createValidDisabledAccessLogKeys() {
	std::set<std::string> keys;
	keys.insert("disable");
	return keys;
}

static std::set<std::string> createValidDisabledErrorLogKeys() {
	std::set<std::string> keys;
	keys.insert("disable");
	return keys;
}

static std::set<std::string> createValidAllowedMethods() {
	std::set<std::string> keys;
	keys.insert("GET");
	keys.insert("POST");
	keys.insert("HEAD");
	keys.insert("DELETE");
	return keys;
}

const std::set<std::string> ConfigParser::VALID_SERVER_KEYS =
	createValidServerKeys();
const std::set<std::string> ConfigParser::VALID_LISTEN_KEYS =
	createValidListenKeys();
const std::set<std::string> ConfigParser::VALID_REDIRECT_KEYS =
	createValidRedirectKeys();
const std::set<std::string> ConfigParser::VALID_LOCATION_KEYS =
	createValidLocationKeys();
const std::set<std::string> ConfigParser::VALID_ACCESS_LOG_KEYS =
	createValidAccessLogKeys();
const std::set<std::string> ConfigParser::VALID_ERROR_LOG_KEYS =
	createValidErrorLogKeys();
const std::set<std::string> ConfigParser::VALID_DISABLED_ACCESS_LOG_KEYS =
	createValidDisabledAccessLogKeys();
const std::set<std::string> ConfigParser::VALID_DISABLED_ERROR_LOG_KEYS =
	createValidDisabledErrorLogKeys();
const std::set<std::string> ConfigParser::VALID_ALLOWED_METHODS =
	createValidAllowedMethods();

void ConfigParser::parseListens(const Node *node) {
	if (!node)
		throw std::runtime_error("Config error: missing 'listens' node");
	const std::vector<Node *> &listensNodes = node->getSeq();
	std::vector<Listen> listens;
	for (std::vector<Node *>::const_iterator it = listensNodes.begin();
		 it != listensNodes.end(); ++it) {
		Node *l_node = *it;
		if (l_node->getKey() != "listen") {
			throw std::runtime_error(
				"Config error: missing 'listen' key in listen item");
		}

		const char *validKeysArr[] = {"interface", "port"};
		std::set<std::string> validKeys(validKeysArr, validKeysArr + 2);
		validateKeys(l_node, validKeys, "listen block");

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

		listens.push_back(l);
	}
	_config->setListens(listens);
}

void ConfigParser::parseRedirects(Node *node) {
	if (!node)
		throw std::runtime_error("Config error: missing 'redirects' node");
	const std::vector<Node *> &redirects = node->getSeq();
	for (std::vector<Node *>::const_iterator it = redirects.begin();
		 it != redirects.end(); ++it) {
		Node *r_node = *it;
		if (r_node->getKey() != "redirect") {
			continue;
		}

		const char *validKeysArr[] = {"from", "to", "code"};
		std::set<std::string> validKeys(validKeysArr, validKeysArr + 3);
		validateKeys(r_node, validKeys, "redirect block");

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

		_config->setRedirect(r, r.fromPath);
	}
}

void ConfigParser::parseServer(const Node *serverNode) {
	ConfigParser::validateKeys(serverNode, ConfigParser::VALID_SERVER_KEYS,
							   "server block");

	parseListens(serverNode->getMapNode("listens"));
	if (Node *redirectsNode = serverNode->getMapNode("redirects")) {
		parseRedirects(redirectsNode);
	}

	ConfigLocationParser locationParser(_config);
	if (Node *locationsNode = serverNode->getMapNode("locations")) {
		locationParser.parseLocations(locationsNode);
	}

	ConfigLogParser logParser(_config);
	if (Node *accessLogsNode = serverNode->getMapNode("access_logs")) {
		logParser.parseAccessLogs(accessLogsNode);
	}
	if (Node *errorLogsNode = serverNode->getMapNode("error_logs")) {
		logParser.parseErrorLogs(errorLogsNode);
	}

	if (Node *n = serverNode->getMapNode("path")) {
		Location defaultLoc = _config->getLocation("/");
		defaultLoc.path = n->getValue();
		_config->setLocation(defaultLoc, "/");
	}
	if (Node *n = serverNode->getMapNode("root"))
		_config->setRoot(n->getValue(), "/");
	if (Node *n = serverNode->getMapNode("allowedMethods")) {
		const char *validMethodsArr[] = {"GET", "POST", "HEAD", "DELETE"};
		std::set<std::string> validMethods(validMethodsArr,
										   validMethodsArr + 4);
		std::set<std::string> methodsSet;
		const std::vector<Node *> &methods = n->getSeq();
		for (std::vector<Node *>::const_iterator m_it = methods.begin();
			 m_it != methods.end(); ++m_it) {
			std::string method = (*m_it)->getValue();
			if (validMethods.find(method) == validMethods.end()) {
				throw std::runtime_error("Config error: invalid HTTP method '" +
										 method + "' in server block");
			}
			methodsSet.insert(method);
		}
		_config->setAllowedMethods(methodsSet, "/");
	}
	if (Node *n = serverNode->getMapNode("autoindex"))
		_config->setAutoindex(n->getValue() == "true", "/");
	if (Node *n = serverNode->getMapNode("indexFile"))
		_config->setIndexFile(n->getValue(), "/");
	if (Node *n = serverNode->getMapNode("errorFile"))
		_config->setErrorFile(n->getValue(), "/");
	if (Node *n = serverNode->getMapNode("uploadStore"))
		_config->setUploadStore(n->getValue(), "/");
	if (Node *n = serverNode->getMapNode("cgi")) {
		const std::vector<std::string> &cgiKeys = n->getKeys();
		for (std::vector<std::string>::const_iterator cgi_it = cgiKeys.begin();
			 cgi_it != cgiKeys.end(); ++cgi_it) {
			_config->setCgiConf(*cgi_it, n->getMapNode(*cgi_it)->getValue(),
								"/");
		}
	}

	if (Node *n = serverNode->getMapNode("maxRequestBodySize"))
		_config->setMaxRequestBodySize(
			StringOps::sizeByteStrToSizeT(n->getValue()));

	if (Node *n = serverNode->getMapNode("timeoutSec"))
		_config->setTimeoutSec(StringOps::stringToInt(n->getValue()));

	if (Node *n = serverNode->getMapNode("maxEvents"))
		_config->setMaxEvents(StringOps::stringToInt(n->getValue()));
}
