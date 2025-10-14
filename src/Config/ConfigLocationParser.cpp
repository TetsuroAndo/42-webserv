#include "ConfigLocationParser.hpp"
#include "../Lib/MyYAML/MyYAML.hpp"
#include "../Lib/StringOps/StringOps.hpp"
#include "Config.hpp"
#include "ConfigParser.hpp"
#include <set>
#include <stdexcept>
#include <vector>

ConfigLocationParser::ConfigLocationParser(Config *config) : _config(config) {}
ConfigLocationParser::~ConfigLocationParser() {}

void ConfigLocationParser::parseLocations(const Node *node) {
	if (!node)
		throw std::runtime_error("Config error: missing 'locations' node");

	const std::vector<Node *> &locations = node->getSeq();
	for (std::vector<Node *>::const_iterator it = locations.begin();
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
				if (ConfigParser::VALID_ALLOWED_METHODS.find(method) ==
					ConfigParser::VALID_ALLOWED_METHODS.end()) {
					throw std::runtime_error(
						"Config error: invalid HTTP method '" + method + "'");
				}
				loc.allowedMethods.insert(method);
			}
		}
		_config->setLocation(loc, loc.path);
	}
}
