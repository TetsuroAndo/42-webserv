#pragma once
#include  "Node.hpp"
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

class MyYAML {
public:
	MyYAML(const std::string &filepath);

	~MyYAML();

	void debugAllKeyAndValue();

	std::vector<std::string> getValue(const std::string &key);

	std::size_t getSize(const std::string &key);

	class FileNotFound : public std::exception {
		const char *what() const throw() { return "File not found"; }
	};

	class InvalidFormat : public std::exception {
		const char *what() const throw() { return "Invalid format"; }
	};

	class ValueNotFound : public std::runtime_error {
	public:
		explicit ValueNotFound(const std::string& key)
			: std::runtime_error("Value not found: " + key) {}
	};

	Node data;
private:
	std::map<std::string, std::vector<std::string> > _myYamlData;
	void parseYaml(std::string buf);
	void NewParseYaml(std::string buf);
};
