#pragma once

class ConfigBuilder;
class Node;

class ConfigLocationParser {
private:
	ConfigBuilder *_builder;
	bool _hasBiggestMaxBodySize;
	unsigned int _biggestMaxBodySize;

public:
	ConfigLocationParser(ConfigBuilder *builder);
	~ConfigLocationParser();

	void parseLocations(const Node *node);
};
