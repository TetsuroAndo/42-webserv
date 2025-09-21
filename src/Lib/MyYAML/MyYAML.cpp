#include "MyYAML.hpp"
#include <algorithm>
#include <cerrno>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stack>

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

static int startCharCount(std::string str, char c) {
	int result = 0;
	std::string::const_iterator it = str.begin();
	const std::string::const_iterator itEnd = str.end();
	while (it != itEnd && c == *it) {
		result++;
		++it;
	}
	return result;
}

static bool isOnlyCharLine(const std::string &line, const char delimiter) {
	std::string::const_iterator it = line.begin();
	const std::string::const_iterator itEnd = line.end();
	while (it != itEnd && ' ' == *it) {
		++it;
	}
	return delimiter == *it;
}

// 前後の空白を削ってくれる関数
static std::string trimWhitespace(const std::string &key) {
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

// keyを抽出する関数
static std::string extractKey(const std::string &line) {
	std::string trimmedLine = trimWhitespace(line);
	// 何もなければ何もないを返す
	if (trimmedLine.empty()) {
		return "";
	}
	// 一文字目がセパレーターならエラー
	if (trimmedLine[0] == ':') {
		throw std::runtime_error("Key is empty");
	}
	const size_t pos = trimmedLine.find(':');
	// セパレーターがなければkeyは何もない
	if (pos == std::string::npos) {
		return "";
	}
	// 前後の空白を取り除いたセパレーターの手前の文字列を返す
	return trimWhitespace(trimmedLine.substr(0, pos));
}

// valueを抽出する関数
static std::string extractValue(const std::string &line) {
	std::string trimmedLine = trimWhitespace(line);
	// 何もなければ何もないを返す
	if (trimmedLine.empty()) {
		return "";
	}
	const size_t pos = trimmedLine.find(':');
	// セパレーターがなければそのまま
	if (pos == std::string::npos) {
		return trimmedLine;
	}
	// 前後の空白を取り除いたセパレーターの後半の文字列を返す
	return trimWhitespace(trimmedLine.substr(pos + 1, trimmedLine.length()));
}

// セパレーターで終わっているかどうかを返す関数
static bool isEndSeparator(const std::string &line) {
	std::string trimmedLine = trimWhitespace(line);
	if (trimmedLine.empty()) {
		return false;
	}
	const size_t pos = trimmedLine.find(':');
	if (pos == std::string::npos) {
		return false;
	}
	if (pos == trimmedLine.length() - 1) {
		return true;
	}
	return false;
}



static std::string extractListValue(const std::string &line) {
	int spaceCount = 0;
	std::string::const_iterator it = line.begin();
	const std::string::const_iterator itEnd = line.end();
	while (it != itEnd && ' ' == *it) {
		spaceCount++;
		++it;
	}
	std::string tmp = trimWhitespace(line);
	if (tmp.size() < 2) {
		throw MyYAML::InvalidFormat();
	}
	if ('-' == tmp[0] && ' ' == tmp[1]) {
		tmp = tmp.substr(2, tmp.size() - 2);
	} else {
		throw MyYAML::InvalidFormat();
	}
	return (tmp);
}

static bool endsWith(const std::string& s, const std::string& suffix) {
	return s.size() >= suffix.size() &&
		   s.compare(s.size() - suffix.size(), suffix.size(), suffix) == 0;
}

MyYAML::MyYAML(const std::string &filepath) : data(NODE_MAP, "root", NULL) {
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

// TODO: 作業終わったらこっち消す
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
	for (; it != endIt; ++it) {
		std::string::const_iterator lineStart = it;
		std::string::const_iterator lineEnd = std::find(it, endIt, '\n');
		std::string line(lineStart, lineEnd);
		it = lineEnd;
		if (line.empty() || isOnlyCharLine(line, '#')) {
			continue;
		}
		if (isOnlyCharLine(line, '-')) {
			if (isPrevKeyOnly) {
				if (!isPrevIsList) {
					_myYamlData[prevKey] = std::vector<std::string>();
					isPrevIsList = true;
				}
				std::string value = extractListValue(line);
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
		const std::string::size_type pos = line.find(":");
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

void MyYAML::NewParseYaml(std::string buf) {
	// TODO: パース処理を完成させる
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
	std::string prevKey = "";
	std::stack<int> prevIndent;
	prevIndent.push(0);
	std::stack<Node> nodeChain;
	MyYamlState nowState = MyYamlState_NONE;
	MyYamlState prevState = MyYamlState_NONE;
	for (; it != endIt; ++it) {
		std::string::const_iterator lineStart = it;
		std::string::const_iterator lineEnd = std::find(it, endIt, '\n');
		std::string line(lineStart, lineEnd);
		it = lineEnd;
		// コメント行・空行
		if (line.empty() || isOnlyCharLine(line, '#')) {
			continue;
		}
		// TODO: ここに書く
		// インデントの数を数える
		int nowIndent = startCharCount(line, ' ');
		// 行の種類特定
		if (isOnlyCharLine(line, '-')) {
			line = extractListValue(line);
			nowState = MyYamlState_SEQ;
		} else {
			nowState = MyYamlState_MAP;
		}
		std::string key = extractKey(line);
		std::string value = extractValue(line);

		// 前回のステートと変化がある場合はインデントの数をprevと比較して正しいかをみる
		if (prevIndent.top() == nowIndent) {
			// インデントに変化がない時に、typeは同じであることを期待
			if (prevState != nowState) {
				throw InvalidFormat();
			}
		} else {
			// 増えたか減ったかで動作を変える
			if (prevIndent.top() < nowIndent) {
				// 階層が深くなった
				// nodeChainにノードを増やす
			} else {
				// 階層が浅くなった
				prevIndent.pop();
				const int expectedIndent = prevIndent.top();
				if (expectedIndent != nowIndent) {
					throw InvalidFormat();
				}
				// インデントが少なくなったタイミングでは、nodeChainのtopを終端処理する
				nodeChain.top().terminateNode();
				nodeChain.pop();
			}
			prevIndent.push(nowIndent);
		}
		// 大丈夫ならデータをしまう
		Node tmpNew(NODE_SEQ, "kokoni key", "kokoni value");
		switch (nowState) {
			case MyYamlState_SEQ:
			tmpNew.setType(NODE_SEQ);
			nodeChain.push(tmpNew);
			break;
			case MyYamlState_MAP:
			tmpNew.setType(NODE_MAP);
			nodeChain.push(tmpNew);
			break;
			default:
				throw InvalidFormat();
		}
		prevState = nowState;

	}
}
