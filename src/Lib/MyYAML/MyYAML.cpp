#include "MyYAML.hpp"
#include <cerrno>
#include <cstring>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>

static std::string readFileAll(const std::string &filepath) {
	std::ifstream input(filepath.c_str());
	if (!input) {
		std::cerr << "Webserv: " << filepath << ": " << strerror(errno) << std::endl;
		throw std::runtime_error("Could not open file");
	}
	std::stringstream buffer;
	buffer << input.rdbuf();
	return buffer.str();
}

static bool isOnlyCommentLine(const std::string &line) {
	std::string::const_iterator it = line.begin();
	const std::string::const_iterator itEnd = line.end();
	while (it != itEnd && ' ' == *it) {
		++it;
	}
	return '#' == *it;
}

static std::string extractKey(const std::string &key) {
	std::string::const_iterator it = key.begin();
	const std::string::const_iterator itEnd = key.end();
	while (it != itEnd && ' ' == *it) {
		++it;
	}
	std::string tmp(it, itEnd);
	std::string::reverse_iterator revIt = tmp.rbegin();
	const std::string::reverse_iterator revEnd = tmp.rend();
	while (revIt != revEnd && ' ' == *revIt) {
		++revIt;
	}
	std::string result(tmp.begin(), revIt.base());
	return result;
}


static std::string extractListValue(const std::string &line, int &prevIndent) {
	int spaceCount = 0;
	std::string::const_iterator it = line.begin();
	const std::string::const_iterator itEnd = line.end();
	while (it != itEnd && ' ' == *it) {
		spaceCount++;
		++it;
	}
	if (prevIndent != -1 && spaceCount != prevIndent) {
		throw MyYAML::InvalidFormat();
	}
	std::string tmp = extractKey(line);
	if (tmp.size() < 2) {
		throw MyYAML::InvalidFormat();
	}
	if ('-' == tmp[0] && ' ' == tmp[1]) {
		tmp = tmp.substr(2, tmp.size() - 2);
	} else {
		throw MyYAML::InvalidFormat();
	}

	prevIndent = spaceCount;
	return (tmp);
}

static bool endsWith(const std::string& s, const std::string& suffix) {
	return s.size() >= suffix.size() &&
		   s.compare(s.size() - suffix.size(), suffix.size(), suffix) == 0;
}

MyYAML::MyYAML(const std::string &filepath) {
	const std::string extension(".yaml");
	if (endsWith(filepath, extension) == false) {
		throw std::invalid_argument("Filepath does not end with extension '" + extension + "'");
	}
	const std::string buf = readFileAll(filepath);
	parseYaml(buf);
}

MyYAML::~MyYAML() {
}

void MyYAML::debugAllKeyAndValue() {
	std::map<std::string, std::vector<std::string> >::const_iterator start =
		_myYamlData.begin();
	const std::map<std::string, std::vector<std::string> >::const_iterator end =
		_myYamlData.end();
	for (; start != end; ++start) {
		std::cout << start->first << ":" << std::endl;
		std::vector<std::string>::const_iterator it = start->second.begin();
		for (; it != start->second.end(); ++it) {
			std::cout << *it << std::endl;
		}
	}

}

std::vector<std::string> MyYAML::getValue(const std::string &key) {
	if (_myYamlData.end() == _myYamlData.find(key)) {
		throw ValueNotFound(key);
	}
	return _myYamlData.find(key)->second;
}

std::size_t MyYAML::getSize(const std::string &key) {
	return getValue(key).size();
}

void MyYAML::parseYaml(std::string buf) {
	if (!buf.empty() && '\n' != buf[buf.size() - 1]) {
		buf.push_back('\n');
	}
	std::string::const_iterator it = buf.begin();
	const std::string::const_iterator endIt = buf.end();
	if (it == endIt) {
		return;
	}
	if ('\n' == *it) {
		++it;
	}
	bool isPrevKeyOnly = false;
	bool isPrevIsList = false;
	std::string prevKey = "";
	int prevIndent = -1;
	for (; it != endIt; ++it) {
		std::string::const_iterator lineStart = it;
		std::string::const_iterator lineEnd = std::find(it, endIt, '\n');
		std::string line(lineStart, lineEnd);
		it = lineEnd;
		if (line.empty() || isOnlyCommentLine(line)) {
			continue;
		}
		const std::string::size_type pos = line.find(":");
		if (std::string::npos == pos) {
			if (isPrevKeyOnly) {
				if (!isPrevIsList) {
					_myYamlData[prevKey] = std::vector<std::string>();
					isPrevIsList = true;
				}
				std::string value = extractListValue(line, prevIndent);
				if (value.empty()) {
					throw InvalidFormat();
				}
				_myYamlData[prevKey].push_back(value);
				continue;
			}
		}
		if (isPrevKeyOnly) {
			// リストのフラグ
			if (!isPrevIsList) {
				//リストを処理せずに抜けてきた
				throw InvalidFormat();
			}
			isPrevKeyOnly = false;
			isPrevIsList = false;
			prevIndent = -1;
		}
		std::string key = line.substr(0, pos);
		key = extractKey(key);
		if (key.empty()) {
			throw InvalidFormat();
		}
		// key:value
		std::string value = line.substr(pos + 1);
		value = extractKey(value);
		if (value.empty()) {
			isPrevKeyOnly = true;
		} else {
			if (_myYamlData[key].empty()) {
				_myYamlData[key] = std::vector<std::string>();
			}
			_myYamlData[key].push_back(value);
		}
		prevKey = key;
	}
	if (isPrevKeyOnly && false == isPrevIsList) {
		throw InvalidFormat();
	}

}

MyYAML::MyYAML() {
}

MyYAML::MyYAML(const MyYAML &other) {
	(void)other;
}

MyYAML &MyYAML::operator=(const MyYAML &other) {
	if (this != &other) {
	}
	return *this;
}
