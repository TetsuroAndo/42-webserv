#pragma once

#include <set>
#include <string>

class ConfigBuilder;
class Node;

class ConfigParser {
private:
	ConfigBuilder *_builder;

public:
	static const std::set< std::string > VALID_SERVER_KEYS;
	static const std::set< std::string > VALID_LISTEN_KEYS;
	static const std::set< std::string > VALID_LOCATION_KEYS;
	static const std::set< std::string > VALID_ACCESS_LOG_KEYS;
	static const std::set< std::string > VALID_ERROR_LOG_KEYS;
	static const std::set< std::string > VALID_DISABLED_ACCESS_LOG_KEYS;
	static const std::set< std::string > VALID_DISABLED_ERROR_LOG_KEYS;
	static const std::set< std::string > VALID_ALLOWED_METHODS;

	static void validateKeys(const Node *node,
							 const std::set< std::string > &validKeys,
							 const std::string &context);

	static size_t validateConvertTimeout(const std::string &configName,
										 const std::string &value);

public:
	ConfigParser(ConfigBuilder *builder);
	~ConfigParser();

	void parseServer(const Node *serverNode) const;
	void parseListens(const Node *node) const;
	void parseErrorPages(const Node *node) const;
};
