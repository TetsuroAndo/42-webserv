#pragma once

class Config;
class Node;

class ConfigLocationParser {
private:
    Config* _config;

public:
    ConfigLocationParser(Config* config);
    ~ConfigLocationParser();

    void parseLocations(const Node *node);
};
