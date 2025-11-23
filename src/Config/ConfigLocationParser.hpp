#pragma once

#include <cstddef>

class ConfigBuilder;
class Node;
class ConfigLocationParser {
private:
	ConfigBuilder *_builder;
	bool _hasBiggestMaxBodySize;
	size_t _biggestMaxBodySize;

public:
	ConfigLocationParser(ConfigBuilder *builder);
	~ConfigLocationParser();

	void parseLocations(const Node *node);
};
