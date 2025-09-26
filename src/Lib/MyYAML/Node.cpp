#include "Node.hpp"
#include "../Colors/Colors.hpp"

#include <algorithm>
#include <iostream>
#include <map>
#include <stdexcept>

Node::Node(const Type type, const std::string &key, const std::string &value) :
	_type(type), _childNodeType(NODE_NULL),
	_key(key),
	_value(value), _isEndSeparator(false),
	_lineIndex(0) {

	if (type == NODE_SEQ && (key == "" && value == "")) {
		throw std::invalid_argument("Key must not be empty");
	}
	if (type == NODE_MAP && key == "") {
		throw std::invalid_argument("Key must not be empty");
	}
	if (type == NODE_VAL && value == "") {
		throw std::invalid_argument("Value must not be empty");
	}
}

Node::~Node() {
	for (std::size_t i = 0; i < _seq.size(); ++i) {
		delete _seq[i];
	}
	for (std::map<std::string, Node *>::const_iterator it = _map.begin()
	     ;
	     it != _map.end(); ++it) {
		delete it->second;
	}
}

void Node::push(Node *node) {
	if (NODE_NULL == _childNodeType) {
		_childNodeType = node->getType();
	}

	const int type = _childNodeType;
	switch (type) {
	case NODE_VAL:
		throw std::runtime_error("Can't push to value node");
	case NODE_SEQ:
		if (node->getType() != NODE_SEQ) {
			throw std::runtime_error("Can't push to sequence node");
		}
		this->_seq.push_back(node);
		break;
	case NODE_MAP:
		if (node->getType() != NODE_MAP) {
			throw std::runtime_error("Can't push to map node");
		}
		if (this->_map.find(node->_key) != this->_map.end()) {
			throw std::runtime_error("Duplicate key " + node->_key);
		}
		this->_map[node->getKey()] = node;
		break;
	default:
		break;
	}
}

void Node::print() const {
	print(2);
}

void Node::print(const int indent) const {
	const std::string ind(indent, ' ');

	if (_childNodeType == NODE_SEQ) {
		std::cout << ind << Colors::RED << Colors::BOLD << "SEQ:" << Colors::RESET
			<< " " << Colors::MAGENTA << _key << Colors::RESET << std::endl;
		for (std::size_t i = 0; i < _seq.size(); i++) {
			_seq[i]->print(indent + 2);
		}
	} else if (_childNodeType == NODE_MAP) {
		std::cout << ind << Colors::GREEN << Colors::BOLD << "MAP:" << Colors::RESET
			<< " " << Colors::MAGENTA << _key << Colors::RESET << std::endl;
		for (std::map<std::string, Node *>::const_iterator it = _map.begin();
		     it != _map.end(); ++it) {
			it->second->print(indent + 2);
		}
	} else {
		if (_key.empty()) {
			std::cout << ind << Colors::BLUE << Colors::BOLD << "VAL:" << Colors::RESET
				<< " " << Colors::CYAN << _value << Colors::RESET << std::endl;
		} else {
			std::cout << ind << Colors::BLUE << Colors::BOLD << "VAL:" << Colors::RESET
				<< " " << Colors::MAGENTA << _key << Colors::RESET
				<< " = " << Colors::CYAN << _value << Colors::RESET << std::endl;
		}
	}
}


bool Node::isValidChildNode() const
{
	switch (_childNodeType) {
	case NODE_SEQ:
		for (std::size_t i = 0; i < _seq.size(); ++i) {
			if (_seq[i]->getKey().empty() && _seq[i]->getValue().empty()) {
				return false;
			}
			if (_seq[i]->isValidNode() == false) {
				return false;
			}
		}
		break;
	case NODE_MAP:
		for (std::map<std::string, Node *>::const_iterator it = _map.begin()
		     ;
		     it != _map.end(); ++it) {
			if (it->second->getKey().empty()) {
				return false;
			}
			if (it->second->isValidNode() == false) {
				return false;
			}
		}
		break;
	default:
		break;
	}
	return true;
}

bool Node::isValidNode() const {
	switch (_type) {
	case NODE_SEQ:
		return isValidChildNode();
	case NODE_MAP:
		return isValidChildNode();
	case NODE_VAL:
		return _value.empty() == false;
	default:
		return false;
	}
}

Type Node::getType() const {
	return _type;
}


const std::string &Node::getKey() const {
	return _key;
}

const std::string &Node::getValue() const {
	return _value;
}

void Node::setTypeValue() {
	_type = NODE_VAL;
	_childNodeType = NODE_VAL;
}

const std::vector<Node *> &Node::getSeq() const {
	if (_childNodeType != NODE_SEQ) {
		throw std::invalid_argument("Node type is not SEQ");
	}
	return _seq;
}

Node *Node::getMapNode(const std::string &key) const {
	if (_childNodeType != NODE_MAP) {
		throw std::invalid_argument("Node type is not MAP");
	}
	if (_map.find(key) == _map.end()) {
		throw std::invalid_argument("Key not found");
	}
	return _map.find(key)->second;
}

void Node::terminateNode() {
	switch (_childNodeType) {
	case NODE_SEQ:
		for (std::size_t i = 0; i < _seq.size(); ++i) {
			_seq[i]->setTypeValue();
			_seq[i]->_childNodeType = NODE_VAL;
		}
		return;
	case NODE_MAP:
		for (std::map<std::string, Node *>::const_iterator it = _map.begin()
		     ;
		     it != _map.end(); ++it) {
			it->second->setTypeValue();
			it->second->_childNodeType = NODE_VAL;
		}
		return;
	default:
		return;
	}
}

// Nodeを整形する(fix(固定)する)
void Node::fixNode() {
	if (_seq.size() == 0 && _map.size() == 0) {
		setTypeValue();
		return;
	}
	switch (_childNodeType) {
	case NODE_SEQ:
		for (std::size_t i = 0; i < _seq.size(); ++i) {
			_seq[i]->fixNode();
		}
		return;
	case NODE_MAP:
		for (std::map<std::string, Node *>::const_iterator it = _map.begin()
		     ;
		     it != _map.end(); ++it) {
			it->second->fixNode();
		}
		return;
	default:
		return;
	}
}

std::size_t Node::size() const {
	switch (_childNodeType) {
	case NODE_MAP:
		return _map.size();
	case NODE_SEQ:
		return _seq.size();
	case NODE_VAL:
		return 1;
	case NODE_NULL:
	default:
		return 0;
	}
}