#pragma once

class ConfigBuilder;
class Node;

class ConfigLogParser {
private:
	ConfigBuilder *_builder;

public:
	ConfigLogParser(ConfigBuilder *builder);
	~ConfigLogParser();

	void parseAccessLogs(const Node *node);
	void parseErrorLogs(const Node *node);
};
