#pragma once

class Config;
class Node;

class ConfigLogParser {
private:
    Config* _config;

public:
    ConfigLogParser(Config* config);
    ~ConfigLogParser();

    void parseAccessLogs(const Node *node);
    void parseErrorLogs(const Node *node);
};
