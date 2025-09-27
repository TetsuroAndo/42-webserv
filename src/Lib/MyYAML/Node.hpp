#pragma once
#include <map>
#include <string>
#include <vector>

enum Type {
	NODE_MAP,
	NODE_SEQ,
	NODE_VAL,
	NODE_NULL
};

class Node {
public:
	Node(const Type type, const std::string &key,
	     const std::string &value);

	~Node();

	void push(Node *node);
	void print() const;
	void print(int indent) const;
	bool isValidChildNode() const;
	bool isValidNode() const;

	Type getType() const;

	const std::string &getKey() const;

	const std::string &getValue() const;

	void setTypeValue();

	const std::vector<Node *> &getSeq() const;
	Node *getMapNode(const std::string &key) const;

	void terminateNode();

	void fixNode();

	std::size_t size() const;

	std::vector<std::string> getKeys() const;

private
:
	Type _type;
	Type _childNodeType;
	const std::string _key;
	const std::string _value;
	std::vector<Node *> _seq;
	std::map<std::string, Node *> _map;
	bool _isEndSeparator;
	int _lineIndex;
};
