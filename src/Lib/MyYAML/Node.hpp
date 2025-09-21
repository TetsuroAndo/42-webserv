#pragma once
#include <map>
#include <string>
#include <vector>

enum Type {
	NODE_MAP,
	NODE_SEQ,
	NODE_VAL
};

class Node {
public:
	Node(const Type type, const std::string &key,
	     const std::string &value) : _type(type), _key(key), _value(value) {

		if (type == NODE_SEQ && key == "") {
			throw std::invalid_argument("Key must not be empty");
		}
		if (type == NODE_MAP && key == "") {
			throw std::invalid_argument("Key must not be empty");
		}
		if (type == VAL && value == "") {
			throw std::invalid_argument("Value must not be empty");
		}
	}
	~Node() {
	};

	void push(Node *node);
	void print(int indent) const;
	bool isValidNode();

	Type getType() const { return _type; }
	const std::string &getKey() const { return _key; }
	const std::string &getValue() const { return _value; }

	std::vector<Node *> getSeq() const {
		if (_type != NODE_SEQ) {
			throw std::invalid_argument("Node type is not SEQ");
		}
		return _seq;
	}

	Node *getMapNode(const std::string &key) const {
		if (_type != NODE_MAP) {
			throw std::invalid_argument("Node type is not MAP");
		}
		if (_map.find(key) == _map.end()) {
			throw std::invalid_argument("Key not found");
		}
		return _map.find(key)->second;
	}

private:
	const Type _type;
	const std::string _key;
	const std::string _value;
	std::vector<Node *> _seq;
	std::map<std::string, Node *> _map;
};