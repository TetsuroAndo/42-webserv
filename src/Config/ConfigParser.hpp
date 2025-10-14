#pragma once

#include <set>
#include <string>

class Config;
class Node;

class ConfigParser {
private:
	Config *_config;

public:
	static const std::set< std::string > VALID_SERVER_KEYS;
	static const std::set< std::string > VALID_LISTEN_KEYS;
	static const std::set< std::string > VALID_REDIRECT_KEYS;
	static const std::set< std::string > VALID_LOCATION_KEYS;
	static const std::set< std::string > VALID_ACCESS_LOG_KEYS;
	static const std::set< std::string > VALID_ERROR_LOG_KEYS;
	static const std::set< std::string > VALID_DISABLED_ACCESS_LOG_KEYS;
	static const std::set< std::string > VALID_DISABLED_ERROR_LOG_KEYS;
	static const std::set< std::string > VALID_ALLOWED_METHODS;

	static void validateKeys(const Node *node,
							 const std::set< std::string > &validKeys,
							 const std::string &context);

public:
	ConfigParser(Config *config);
	~ConfigParser();

	void parseServer(const Node *serverNode);
	void parseListens(const Node *node);
	void parseRedirects(Node *node);
};
