#pragma once
#include <map>
#include <string>
#include <vector>

enum Type {
	MAP,
	SEQ,
	VAL
};

class Node {
public:
	Node() : _type(VAL) {}
	Node(const Type type, const std::string &key, const std::string &value) : _type(type), _key(key), _value(value) {}
	~Node() {};

	void push(Node *node);
	void print(int indent) const;
	bool isValidNode();

	Type getType() const { return _type; }
	const std::string &getKey() const { return _key; }
	const std::string &getValue() const { return _value; }
	std::vector<Node *> getSeq() const { return _seq; }
	std::map<std::string, Node *> getMap() const { return _map; }
private:
	Type _type;
	std::string _key;
	std::string _value;
	std::vector<Node *> _seq;
	std::map<std::string, Node *> _map;
};