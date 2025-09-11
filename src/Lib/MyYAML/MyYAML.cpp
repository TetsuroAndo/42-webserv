#include "MyYAML.hpp"

#include <fstream>
#include <sstream>
#include <algorithm>
#include <iostream>

std::string MyYAML::readFileAll(const std::string &filepath) {
	std::ifstream input(filepath.c_str());
	std::stringstream buffer;
	buffer << input.rdbuf();
	return buffer.str();
}

static bool isOnlyCommentLine(const std::string &line) {
	std::string::const_iterator it = line.begin();
	const std::string::const_iterator itEnd = line.end();
	while (it != itEnd && isspace(*it)) {
		++it;
	}
	return *it == '#';
}

static std::string extractKey(const std::string &key) {
	// TODO:入力stringの前後の空白を取り除く
	return (key);
}

static std::string extractListValue(const std::string & line, int prevIndent) {
	// TODO:ブロックのインデントの空白の数が揃っているかを検証
	// TODO:extractKeyで前後のゴミを取り除く
	// TODO:前についてるはずのハイフンを検証+取り除く
	(void)prevIndent;
	return (line);
}


void MyYAML::parseYaml(std::string buf) {
	if (!buf.empty() && buf[buf.size() - 1] != '\n') {
		buf.push_back('\n');
	}
	std::string::const_iterator it = buf.begin();
	const std::string::const_iterator endIt = buf.end();
	if (it == endIt) {
		return;
	}
	if (*it == '\n') {
		++it;
	}
	bool isPrevKeyOnly = false;
	bool isPrevIsList = false;
	std::string prevKey = "";
	int prevIndent = 0;
	for (; it != endIt; ++it) {
		std::string line = "";
		while (*it != '\n' && it != endIt) {
			line += *it;
			++it;
		}
		if (line.empty() || isOnlyCommentLine(line)) {
			continue;
		}
		const std::string::size_type pos = line.find(":");
		if (pos == std::string::npos) {
			// TODO:インデントの数を数える
			// TODO:ハイフンがあるかどうかを確かめる
			if (isPrevKeyOnly) {
				if (!isPrevIsList) {
					myYamlData[prevKey] = std::vector<std::string>();
					isPrevIsList = true;
				}
				std::string value = extractListValue(line, prevIndent);
				if (value.empty()) {
					throw InvalidFormat();
				}
				myYamlData[prevKey].push_back(value);
				continue;
			}
		}
		if (isPrevKeyOnly) { // リストのフラグ
			if (!isPrevIsList) { //リストを処理せずに抜けてきた
				throw InvalidFormat();
			}
			isPrevKeyOnly = false;
			isPrevIsList = false;
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
			if (myYamlData[key].empty()) {
				myYamlData[key] =  std::vector<std::string>();
			}
			myYamlData[key].push_back(value);
		}
		prevKey = key;
	}

}

MyYAML::MyYAML()
{

}

MyYAML::MyYAML(const std::string &filepath) {
	const std::string buf = readFileAll(filepath);
	parseYaml(buf);
}

MyYAML::~MyYAML()
{
}

void MyYAML::debugAllKeyAndValue() {
	std::map<std::string, std::vector<std::string> >::const_iterator start = myYamlData.begin();
	const std::map<std::string, std::vector<std::string> >::const_iterator end = myYamlData.end();
	for (; start != end; ++start) {
		std::cout << start->first << ":" << std::endl;
		std::vector<std::string>::const_iterator it = start->second.begin();
		for (; it != start->second.end(); ++it) {
			std::cout << *it << std::endl;
		}
	}

}

MyYAML::MyYAML(const MyYAML& other)
{
	(void)other;
}

MyYAML& MyYAML::operator=(const MyYAML& other)
{
    if (this != &other)
    {
    }
    return *this;
}