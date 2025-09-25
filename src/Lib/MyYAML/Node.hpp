#pragma once
#include <map>
#include <stdexcept>
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
		if (type == NODE_VAL && value == "") {
			throw std::invalid_argument("Value must not be empty");
		}
	}
	~Node() {
		switch (_type) {
		case NODE_SEQ:
			for (std::size_t i = 0; i < _seq.size(); ++i) {
				delete _seq[i];
			}
			return;
		case NODE_MAP:
			for (std::map<std::string, Node *>::const_iterator it = _map.begin();
				 it != _map.end(); ++it) {
				delete it->second;
				 }
			return;
		case NODE_VAL:
			return;
		}
	};

	void push(Node *node);
	void print(int indent) const;
	bool isValidNode();

	Type getType() const { return _type; }
	const std::string &getKey() const { return _key; }
	const std::string &getValue() const { return _value; }

	void setTypeValue() {
		_type = NODE_VAL;
	}

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

	void terminateNode();

private:
	Type _type;
	const std::string _key;
	const std::string _value;
	std::vector<Node *> _seq;
	std::map<std::string, Node *> _map;
};