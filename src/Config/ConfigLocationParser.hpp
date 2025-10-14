#pragma once

class ConfigBuilder;
class Node;

class ConfigLocationParser {
private:
	ConfigBuilder *_builder;

public:
	ConfigLocationParser(ConfigBuilder *builder);
	~ConfigLocationParser();

	void parseLocations(const Node *node);
};
