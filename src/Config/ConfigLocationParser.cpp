#include "ConfigLocationParser.hpp"
#include "../Http/Core/HttpStatus.hpp"
#include "../Lib/MyYAML/MyYAML.hpp"
#include "../Lib/StringOps/StringOps.hpp"
#include "Config.hpp"
#include "ConfigBuilder.hpp"
#include "ConfigParser.hpp"

#include <cerrno>
#include <cstring>
#include <limits>
#include <set>
#include <stdexcept>
#include <unistd.h>
#include <vector>

namespace {
const char *const VALID_BOOL_VALUES[] = {"true", "false", "on",
										 "off",	 "yes",	  "no"};
const size_t VALID_BOOL_VALUES_SIZE =
	sizeof(VALID_BOOL_VALUES) / sizeof(VALID_BOOL_VALUES[0]);

void isValidBoolString(const std::string &value) {

	bool flag = false;
	for (size_t i = 0; i < VALID_BOOL_VALUES_SIZE; ++i) {
		if (VALID_BOOL_VALUES[i] == value) {
			flag = true;
			break;
		}
	}
	if (!flag) {
		throw std::runtime_error("Config error: invalid value '" + value +
								 "' in 'location' ");
	}
}

} // namespace

ConfigLocationParser::ConfigLocationParser(ConfigBuilder *builder)
	: _builder(builder), _hasBiggestMaxBodySize(false), _biggestMaxBodySize(0) {
}
ConfigLocationParser::~ConfigLocationParser() {}

void ConfigLocationParser::parseLocations(const Node *node) {
	if (!node)
		throw std::runtime_error("Config error: missing 'locations' node");

	const std::vector< Node * > &locations = node->getSeq();
	for (std::vector< Node * >::const_iterator it = locations.begin();
		 it != locations.end(); ++it) {
		Node *l_node = *it;
		if (l_node->getKey() != "location") {
			continue;
		}

		ConfigParser::validateKeys(l_node, ConfigParser::VALID_LOCATION_KEYS,
								   "location block");

		Location loc;
		Node *pathNode = l_node->getMapNode("path");
		if (!pathNode)
			throw std::runtime_error(
				"Config error: missing 'path' key in location item");
		loc.path = pathNode->getValue();

		Node *rootNode = l_node->getMapNode("root");
		if (rootNode)
			loc.root = rootNode->getValue();
		Node *uploadStoreNode = l_node->getMapNode("uploadStore");
		if (uploadStoreNode)
			loc.uploadStore = uploadStoreNode->getValue();
		Node *indexNode = l_node->getMapNode("index");
		if (indexNode)
			loc.index = indexNode->getValue();
		Node *directoryErrorNode = l_node->getMapNode("directoryError");
		if (directoryErrorNode)
			loc.directoryError = directoryErrorNode->getValue();
		Node *autoindexNode = l_node->getMapNode("autoindex");
		if (autoindexNode) {
			std::string value = autoindexNode->getValue();
			isValidBoolString(value);
			loc.autoindex =
				(value == "true" || value == "on" || value == "yes");
		}

		Node *returnNode = l_node->getMapNode("return");
		if (returnNode) {
			loc.hasRedirect = true;
			std::string returnValue = returnNode->getValue();
			std::istringstream iss(returnValue);
			std::string codeStr;
			std::string urlStr;

			if (!(iss >> codeStr) || !(iss >> urlStr)) {
				throw std::runtime_error(
					"Config error: invalid 'return' directive in location " +
					loc.path);
			}
			loc.redirectCode = StringOps::stringToInt(codeStr);
			loc.redirectUrl = urlStr;

			if (HttpStatus::isValidStatusCode(loc.redirectCode, 300, 400) ==
				false) {
				throw std::runtime_error("Config error: invalid redirect code "
										 "in 'return' directive");
			}
		}

		Node *maxRequestBodySizeNode = l_node->getMapNode("maxRequestBodySize");
		if (maxRequestBodySizeNode) {
			size_t sizeValue = StringOps::sizeByteStrToSizeT(
				maxRequestBodySizeNode->getValue());
			loc.hasMaxRequestBodySize = true;
			loc.maxRequestBodySize = sizeValue;
			if (!_hasBiggestMaxBodySize ||
				_biggestMaxBodySize < loc.maxRequestBodySize) {
				_hasBiggestMaxBodySize = true;
				_biggestMaxBodySize = loc.maxRequestBodySize;
			}
		}

		loc.allowedMethods = ConfigParser::VALID_ALLOWED_METHODS;

		Node *sessionNode = l_node->getMapNode("session");
		if (sessionNode) {
			std::string value = sessionNode->getValue();
			isValidBoolString(value);
			loc.session = (value == "true" || value == "on" || value == "yes");
		}

		if (Node *allowMethodsNode = l_node->getMapNode("allowedMethods")) {
			loc.allowedMethods.clear();

			const std::vector< Node * > &methods = allowMethodsNode->getSeq();
			for (std::vector< Node * >::const_iterator m_it = methods.begin();
				 m_it != methods.end(); ++m_it) {
				std::string method = (*m_it)->getValue();
				if (ConfigParser::VALID_ALLOWED_METHODS.find(method) ==
					ConfigParser::VALID_ALLOWED_METHODS.end()) {
					throw std::runtime_error(
						"Config error: invalid HTTP method '" + method + "'");
				}
				loc.allowedMethods.insert(method);
			}
		}
		Node *interpreterNode = l_node->getMapNode("interpreterPath");
		if (interpreterNode) {
			const std::vector< std::string > &keys = interpreterNode->getKeys();
			for (std::vector< std::string >::const_iterator keyIt =
					 keys.begin();
				 keyIt != keys.end(); ++keyIt) {
				const std::string &ext = *keyIt;
				Node *interpreterPathNode = interpreterNode->getMapNode(ext);
				if (interpreterPathNode) {
					if (ext[0] != '.') {
						throw std::runtime_error(
							"Config error: invalid Interpreter extension");
					}
					loc.cgiConf[ext] = interpreterPathNode->getValue();
				}
			}
		}

		Node *chunkedTimeoutNode = l_node->getMapNode("chunkedTimeout");
		if (chunkedTimeoutNode) {
			loc.chunkedTimeoutSec = ConfigParser::validateConvertTimeout(
				"chunkedTimeout", chunkedTimeoutNode->getValue());
			loc.hasChunkedTimeoutSec = true;
		}
		_builder->setLocation(loc);
	}
	_builder->setBiggestRequestBodySize(_hasBiggestMaxBodySize,
										_biggestMaxBodySize);
}
