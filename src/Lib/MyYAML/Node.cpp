#include "Node.hpp"

#include <algorithm>
#include <iostream>
#include <map>
#include <stdexcept>

void Node::push(Node *node) {
	if (NODE_NULL == _childNodeType) {
		_childNodeType = node->getType();
	}

	switch (int type = _childNodeType) {
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
	if (_childNodeType == NODE_SEQ) {
		std::cout << ind << "SEQ: " << _key << std::endl;
		for (std::size_t i = 0; i < _seq.size(); i++) {
			_seq[i]->print(indent + 2);
		}
	} else if (_childNodeType == NODE_MAP) {
		std::cout << ind << "MAP: " << _key << std::endl;
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
		if (_seq.size() == 0 && _map.size() == 0) {
			return false;
		}
		for (std::size_t i = 0; i < _seq.size(); ++i) {
			if (_seq[i]->getKey().empty() && _seq[i]->getValue().empty()) {
				return false;
			}
			if (_seq[i]->isValidNode() == false) {
				return false;
			}
		}
		for (std::map<std::string, Node *>::const_iterator it = _map.begin()
	 ;
	 it != _map.end(); ++it) {
			if (it->second->getKey().empty() && it->second->getValue().empty()) {
				return false;
			}
			if (it->second->isValidNode() == false) {
				return false;
			}
	 }
		return true;
	case NODE_MAP:
		if (_seq.size() == 0 && _map.size() == 0) {
			return false;
		}
		for (std::size_t i = 0; i < _seq.size(); ++i) {
			if (_seq[i]->getKey().empty()) {
				return false;
			}
			if (_seq[i]->isValidNode() == false) {
				return false;
			}
		}
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
		return true;
	case NODE_VAL:
		return _value.empty() == false;
	default:
		return false;

	}
}

void Node::terminateNode() {
	switch (_type) {
	case NODE_SEQ:
		for (std::map<std::string, Node *>::const_iterator it = _map.begin()
		     ;
		     it != _map.end(); ++it) {
			it->second->setTypeValue();
			it->second->_childNodeType = NODE_VAL;
		}
		for (std::size_t i = 0; i < _seq.size(); ++i) {
			_seq[i]->setTypeValue();
			_seq[i]->_childNodeType = NODE_VAL;
		}
		return;
	case NODE_MAP:
		for (std::size_t i = 0; i < _seq.size(); ++i) {
			_seq[i]->setTypeValue();
			_seq[i]->_childNodeType = NODE_VAL;
		}
		for (std::map<std::string, Node *>::const_iterator it = _map.begin()
		     ;
		     it != _map.end(); ++it) {
			it->second->setTypeValue();
			it->second->_childNodeType = NODE_VAL;
		}
		return;
	case NODE_VAL:
		throw std::runtime_error("Node is already terminated");
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
	switch (_type) {
	case NODE_SEQ:
		for (std::map<std::string, Node *>::const_iterator it = _map.begin()
			 ;
			 it != _map.end(); ++it) {
				it->second->fixNode();
			 }
		for (std::size_t i = 0; i < _seq.size(); ++i) {
			_seq[i]->fixNode();
		}
		return;
	case NODE_MAP:
		for (std::size_t i = 0; i < _seq.size(); ++i) {
			_seq[i]->fixNode();
		}
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