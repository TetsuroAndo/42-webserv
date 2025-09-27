#pragma once
#include "Node.hpp"
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

enum MyYamlState {
	MyYamlState_NONE,
	MyYamlState_SEQ,
	MyYamlState_MAP,
	MyYamlState_VAL
};

class MyYAML {
public:
	MyYAML(const std::string &filepath);

	~MyYAML();

	class FileNotFound : public std::exception {
		const char *what() const throw() { return "File not found"; }
	};

	class InvalidFormat : public std::exception {
		const char *what() const throw() { return "Invalid format"; }
	};

	class ValueNotFound : public std::runtime_error {
	public:
		explicit ValueNotFound(const std::string &key)
			: std::runtime_error("Value not found: " + key) {}
	};

	Node &getData() const;

private:
	Node *_data;
	void parseYaml(std::string buf);
};
