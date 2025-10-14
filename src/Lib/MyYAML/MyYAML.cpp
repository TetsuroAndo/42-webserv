#include "MyYAML.hpp"
#include "../StringOps/StringOps.hpp"

#include <cerrno>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stack>

namespace /* throws */
{
void throwInvalidFormat(const int line) {
	std::ostringstream oss;
	oss << "Invalid Format at line " << (line + 1);
	throw std::runtime_error(oss.str());
}

void throwInvalidFormat(const int line, const std::string &message) {
	std::ostringstream oss;
	oss << "Invalid Format at line " << (line + 1) << ": " << message;
	throw std::runtime_error(oss.str());
}
} // namespace

namespace /* helper functions */
{
std::string readFileAll(const std::string &filepath) {
	std::ifstream input(filepath.c_str());
	if (!input) {
		std::cerr << "Webserv: " << filepath << ": " << strerror(errno)
				  << std::endl;
		throw std::runtime_error("Could not open file");
	}
	std::stringstream buffer;
	buffer << input.rdbuf();
	return buffer.str();
}

// keyを抽出する関数
std::string extractKey(const std::string &line) {
	std::string trimmedLine = line;
	StringOps::trim(trimmedLine, " ");
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
	std::string result = trimmedLine.substr(0, pos);
	StringOps::trim(result, " ");
	return result;
}

// valueを抽出する関数
std::string extractValue(const std::string &line) {
	std::string trimmedLine = line;
	StringOps::trim(trimmedLine, " ");
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
	std::string result = trimmedLine.substr(pos + 1, trimmedLine.length());
	StringOps::trim(result, " ");
	return result;
}

// セパレーターで終わっているかどうかを返す関数
bool isEndSeparator(const std::string &line) {
	std::string trimmedLine = line;
	StringOps::trim(trimmedLine, " ");
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

std::string extractListValue(const std::string &line) {
	std::string::const_iterator it = line.begin();
	const std::string::const_iterator itEnd = line.end();
	while (it != itEnd && ' ' == *it) {
		++it;
	}
	std::string tmp = line;
	StringOps::trim(tmp, " ");
	if (tmp.size() < 2) {
		throw std::runtime_error("Invalid Format");
	}
	if ('-' == tmp[0] && ' ' == tmp[1]) {
		tmp = tmp.substr(2, tmp.size() - 2);
	} else {
		throw std::runtime_error("Invalid Format");
	}
	return tmp;
}
} // Anonymous namespace

MyYAML::MyYAML(const std::string &filepath) {
	_data = NULL;
	const std::string extension(".yaml");
	if (StringOps::endsWith(filepath, extension) == false) {
		throw std::invalid_argument("Filepath does not end with extension '" +
									extension + "'");
	}
	const std::string buf = readFileAll(filepath);
	parseYaml(buf);
}

MyYAML::~MyYAML() {
	if (_data != NULL) {
		delete _data;
	}
}

Node &MyYAML::getData() const {
	if (_data == NULL) {
		throw std::invalid_argument("Data is null");
	}
	return *_data;
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
	std::stack< int > prevIndent;
	std::stack< Node * > nodeChain;
	MyYamlState nowState = MyYamlState_NONE;
	std::stack< MyYamlState > prevStates;
	Node *rootNode = NULL;

	prevIndent.push(-1);
	prevStates.push(MyYamlState_NONE);

	std::vector< std::string > lines;
	{
		std::istringstream iss(buf);
		std::string l;
		while (std::getline(iss, l)) {
			lines.push_back(l);
		}
	}

	for (size_t idx = 0; idx < lines.size(); ++idx) {
		std::string line = lines[idx];
		// コメント行・空行
		if (line.empty() || StringOps::isOnlyCharLine(line, '#')) {
			continue;
		}
		// インデントの数を数える
		int nowIndent = StringOps::startCharCount(line, ' ');

		// 行の種類特定
		if (StringOps::isOnlyCharLine(line, '-')) {
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
			if (prevStates.top() != nowState) {
				delete rootNode;
				throwInvalidFormat(idx);
			}
		} else {
			// 増えたか減ったかで動作を変える
			if (prevIndent.top() < nowIndent) {
				// 階層が深くなった
				// スタックに乗せる
				prevIndent.push(nowIndent);
				prevStates.push(nowState);
			} else {
				// 階層が浅くなった
				// nodeChainのtopを終端処理する
				nodeChain.top()->terminateNode();
				// インデントの階層が浅くなるまでポップする
				while (1 < prevIndent.size() && prevIndent.top() > nowIndent) {
					prevIndent.pop();
					prevStates.pop();
					nodeChain.pop();
				}

				// ポップ後のインデントが現在のインデントと一致するか確認
				if (prevIndent.top() != nowIndent) {
					delete rootNode;
					throwInvalidFormat(idx);
				}

				// ステートの検証
				if (prevStates.top() != nowState) {
					delete rootNode;
					throwInvalidFormat(idx);
				}
			}
		}

		Type newNodeType;
		// newNodeTypeを設定
		if (MyYamlState_SEQ == nowState) {
			newNodeType = NODE_SEQ;
		} else {
			newNodeType = NODE_MAP;
		}
		Node *newNode = new Node(newNodeType, key, value);
		// 1個目の要素で一階層目のTypeを決定
		if (nodeChain.empty()) {
			rootNode = new Node(newNodeType, "root", "");
			nodeChain.push(rootNode);
		}
		try {
			nodeChain.top()->push(newNode);
		} catch (const std::exception &e) {
			delete rootNode;
			throwInvalidFormat(idx, e.what());
		}

		if (idx < lines.size() - 1) {
			// 次インデントが増える場合
			int nextIndent = StringOps::startCharCount(lines[idx + 1], ' ');
			if (nowIndent < nextIndent) {
				// valueがあった場合はエラー
				if (isEndSeparator(line) == false) {
					delete rootNode;
					throwInvalidFormat(idx);
				}
				// nodeChainにtmpNodeを追加する
				nodeChain.push(newNode);
			}
		}
	}
	if (rootNode != NULL) {
		rootNode->fixNode();
		if (rootNode->isValidNode() == false) {
			delete rootNode;
			throw std::runtime_error("Invalid format");
		}
	}
	_data = rootNode;
}
