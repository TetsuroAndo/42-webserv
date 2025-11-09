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
const char *const VALID_AUTOINDEX_VALUES[] = {"true", "false", "on",
											  "off",  "yes",   "no"};
const size_t VALID_AUTOINDEX_VALUES_SIZE =
	sizeof(VALID_AUTOINDEX_VALUES) / sizeof(VALID_AUTOINDEX_VALUES[0]);
} // namespace

ConfigLocationParser::ConfigLocationParser(ConfigBuilder *builder)
	: _builder(builder), _biggestMaxBodySize(-1) {}
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
			bool flag = false;
			for (size_t i = 0; i < VALID_AUTOINDEX_VALUES_SIZE; ++i) {
				if (VALID_AUTOINDEX_VALUES[i] == value) {
					flag = true;
					break;
				}
			}
			if (!flag) {
				throw std::runtime_error("Config error: invalid value '" +
										 value + "' in 'location' ");
			}
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
			if (sizeValue >
				static_cast< size_t >(std::numeric_limits< int >::max())) {
				throw std::runtime_error(
					"Config error: maxRequestBodySize value is too large "
					"in 'location/maxRequestBodySize' directive");
			}
			loc.maxRequestBodySize = static_cast< int >(sizeValue);
			if (_biggestMaxBodySize < loc.maxRequestBodySize) {
				_biggestMaxBodySize = loc.maxRequestBodySize;
			}
		}

		loc.allowedMethods = ConfigParser::VALID_ALLOWED_METHODS;

		Node *sessionNode = l_node->getMapNode("session");
		if (sessionNode)
			loc.session = (sessionNode->getValue() == "true");

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
			for (std::vector< std::string >::const_iterator it = keys.begin();
				 it != keys.end(); ++it) {
				const std::string &ext = *it;
				Node *pathNode = interpreterNode->getMapNode(ext);
				if (pathNode) {
					loc.cgiConf[ext] = pathNode->getValue();
				}
			}
		}
		_builder->setLocation(loc);
	}
	_builder->setBiggestRequestBodySize(_biggestMaxBodySize);
}
