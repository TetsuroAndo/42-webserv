#include "Node.hpp"

#include <algorithm>
#include <iostream>
#include <map>
#include <stdexcept>

void Node::push(Node *node) {
	switch (int type = _type) {
	case NODE_VAL:
		throw std::runtime_error("Can't push to value node");
	case NODE_SEQ:
		this->_seq.push_back(node);
		break;
	case NODE_MAP:
		if (this->_map.find(node->_key) != this->_map.end()) {
			throw std::runtime_error("Duplicate key " + node->_key);
		}
		this->_map[node->getKey()] = node;
		break;
	default:
		break;
	}
}

void Node::print(const int indent) const {
	const std::string ind(indent, ' ');
	if (_type == NODE_SEQ) {
		std::cout << ind << "SEQ: " << _value << std::endl;
		for (std::size_t i = 0; i < _seq.size(); i++) {
			_seq[i]->print(indent + 2);
		}
	} else if (_type == NODE_MAP) {
		std::cout << ind << "MAP: " << _value << std::endl;
		std::map<std::string, Node *>::const_iterator it = _map.begin();
		const std::map<std::string, Node *>::const_iterator itEnd = _map.end();
		for (; it != itEnd; ++it) {
			std::cout << ind << "KEY: " << it->first << std::endl;
			it->second->print(indent + 2);
		}
	} else {
		std::cout << ind << "VAL: " << _key << " = " << _value << std::endl;
	}
}

bool Node::isValidNode() {
	switch (_type) {
	case NODE_SEQ:
		for (std::size_t i = 0; i < _seq.size(); ++i) {
			if (!_seq[i]->isValidNode())
				return false;
		}
		return true;
	case NODE_MAP: {
		for (std::map<std::string, Node *>::const_iterator it = _map.begin();
		     it != _map.end(); ++it) {
			if (!it->second->isValidNode())
				return false;
		}
		return true;
	}
	case NODE_VAL:
		return true;
	default:
		return false;
	}
}

void Node::terminateNode() {
	switch (_type) {
		case NODE_SEQ:
		for (std::size_t i = 0; i < _seq.size(); ++i) {
			_seq[i]->setTypeValue();
		}
		return;
		case NODE_MAP:
		for (std::map<std::string, Node *>::const_iterator it = _map.begin();
			 it != _map.end(); ++it) {
			it->second->setTypeValue();
		}
		return;
		case NODE_VAL:
			throw std::runtime_error("Node is already terminated");
		return;

	}
}