#include <cstdlib>
#include <iostream>
#include <sstream>

#include "../Lib/URI/uri.hpp"
#include "HttpRequest.hpp"

HttpRequest::HttpRequest() : _complete(false) {}

HttpRequest::~HttpRequest() {}

bool HttpRequest::parse(std::string &buffer) {
	if (_complete) {
		return true;
	}

	size_t headerEnd = buffer.find("\r\n\r\n");
	if (headerEnd == std::string::npos) {
		return false;
	}

	std::string headerPart = buffer.substr(0, headerEnd);
	std::istringstream headerStream(headerPart);
	std::string requestLine;

	if (!std::getline(headerStream, requestLine)) {
		return false;
	}
	if (!HttpRequest::parseRequestLine(requestLine)) {
		return false;
	}
	if (!HttpRequest::parseHeaders(headerStream)) {
		return false;
	}
	if (!HttpRequest::parseBody(buffer, headerEnd + 4)) {
		return false;
	}
	return _complete;
}

static void trimCR(std::string &line) {
	if (!line.empty() && line[line.size() - 1] == '\r') {
		line.erase(line.size() - 1);
	}
}

bool HttpRequest::parseRequestLine(std::string &requestLine) {
	trimCR(requestLine);
	if (requestLine.empty()) {
		return false;
	}
	if (!splitRequestLine(requestLine)) {
		return false;
	}
	splitPathAndQuery();
	parseQueryString();
	return true;
}

bool HttpRequest::splitRequestLine(const std::string &requestLine) {
	size_t firstSpace = requestLine.find(' ');
	size_t lastSpace = requestLine.rfind(' ');
	if (firstSpace == std::string::npos || lastSpace == std::string::npos ||
		firstSpace == lastSpace) {
		return false;
	}

	_method = requestLine.substr(0, firstSpace);
	if (_method != "GET" && _method != "POST" && _method != "DELETE") {
		return false;
	}

	_path = requestLine.substr(firstSpace + 1, lastSpace - firstSpace - 1);
	if (_path.find(' ') != std::string::npos) {
		return false;
	}

	_version = requestLine.substr(lastSpace + 1);
	if (_version != "HTTP/1.0" && _version != "HTTP/1.1") {
		return false;
	}

	return true;
}

void HttpRequest::splitPathAndQuery() {
	size_t questionPos = _path.find('?');
	std::string decodedPath;
	if (questionPos != std::string::npos) {
		decodedPath = URI::decodeURIComponent(_path.substr(0, questionPos));
		_queryString = _path.substr(questionPos + 1);
	} else {
		decodedPath = URI::decodeURIComponent(_path);
		_queryString.clear();
	}
	_path = decodedPath;
}

void HttpRequest::parseQueryString() {
	_query.clear();
	if (_queryString.empty()) {
		return;
	}

	std::istringstream iss(_queryString);
	std::string pair;
	while (std::getline(iss, pair, '&')) {
		size_t equalPos = pair.find('=');
		std::string key;
		std::string value;
		if (equalPos != std::string::npos) {
			key = pair.substr(0, equalPos);
			value = pair.substr(equalPos + 1);
		} else {
			key = pair;
			value = "";
		}
		_query[URI::decodeURIComponent(key)] = URI::decodeURIComponent(value);
	}
}

static void trimSpaces(std::string &s, std::string spaces) {
	size_t start = s.find_first_not_of(spaces);
	size_t end = s.find_last_not_of(spaces);
	if (start == std::string::npos) {
		s.clear();
	} else {
		s = s.substr(start, end - start + 1);
	}
}

static void toLower(std::string &str) {
	for (std::string::size_type i = 0; i < str.size(); ++i) {
		str[i] =
			static_cast<char>(std::tolower(static_cast<unsigned char>(str[i])));
	}
}

bool HttpRequest::parseHeaders(std::istringstream &headerStream) {
	size_t headerBytes = 0;
	std::string line;
	while (std::getline(headerStream, line)) {
		headerBytes += line.size() + 1;
		if (headerBytes > maxHeaderSize) {
			return false;
		}

		trimCR(line);

		size_t pos = line.find(":");
		if (pos != std::string::npos) {
			std::string key = line.substr(0, pos);
			toLower(key);
			std::string value = line.substr(pos + 1);
			trimSpaces(value, " \t");
			if (_headers.find(key) != _headers.end()) {
				return false;
			}
			_headers[key] = value;
		}
	}
	return true;
}

bool HttpRequest::parseBody(std::string &buffer, size_t bodyStart) {
	if (_headers.find("transfer-encoding") != _headers.end() &&
		_headers["transfer-encoding"].find("chunked") != std::string::npos) {
		return parseChunkedBody(buffer, bodyStart);
	}
	if (_headers.find("content-length") != _headers.end()) {
		return parseContentLengthBody(buffer, bodyStart);
	}
	_body = "";
	_complete = true;
	buffer.erase(0, bodyStart);
	return true;
}

static bool getChunkSize(const std::string &buffer, size_t pos, long &chunkSize,
						 size_t &nextPos) {
	size_t crlf = buffer.find("\r\n", pos);
	if (crlf == std::string::npos) {
		return false;
	}

	std::string chunkSizeStr = buffer.substr(pos, crlf - pos);
	size_t semiPos = chunkSizeStr.find(";");
	if (semiPos != std::string::npos)
		chunkSizeStr = chunkSizeStr.substr(0, semiPos);

	char *endptr = NULL;
	chunkSize = std::strtol(chunkSizeStr.c_str(), &endptr, 16);
	if (endptr == chunkSizeStr.c_str() || chunkSize < 0) {
		return false;
	}

	nextPos = crlf + 2;
	return true;
}

static bool readChunkData(const std::string &buffer, size_t &pos,
						  std::string &body, long chunkSize,
						  size_t maxBodySize) {
	if (pos + static_cast<size_t>(chunkSize) + 2 > buffer.size()) {
		return false;
	}
	if (body.size() + static_cast<size_t>(chunkSize) > maxBodySize) {
		return false;
	}

	body.append(buffer, pos, chunkSize);
	pos += chunkSize;

	if (buffer.compare(pos, 2, "\r\n") != 0)
		return false;
	pos += 2;
	return true;
}

static size_t handleLastChunk(const std::string &buffer, size_t pos) {
	size_t trailerEnd = buffer.find("\r\n\r\n", pos);

	if (buffer.compare(pos, 2, "\r\n") == 0) {
		pos += 2;
	} else if (trailerEnd != std::string::npos) {
		pos = trailerEnd + 4;
	}
	return std::min(pos, buffer.size());
}

bool HttpRequest::parseChunkedBody(std::string &buffer, size_t bodyStart) {
	size_t erasePos = bodyStart;
	size_t pos = bodyStart;

	while (true) {
		long chunkSize;
		size_t nextPos;
		if (!getChunkSize(buffer, pos, chunkSize, nextPos)) {
			return false;
		}
		pos = nextPos;
		if (chunkSize == 0) {
			erasePos = std::min(handleLastChunk(buffer, pos), buffer.size());
			_complete = true;
			break;
		}
		if (!readChunkData(buffer, pos, _body, chunkSize, maxBodySize)) {
			return false;
		}
		erasePos = pos;
	}

	if (erasePos > 0)
		buffer.erase(0, erasePos);
	return true;
}

bool HttpRequest::parseContentLengthBody(std::string &buffer,
										 size_t bodyStart) {
	std::string lenStr = _headers["content-length"];
	trimSpaces(lenStr, " \t\n\r\f\v");

	char *endPtr = NULL;
	long contentLength = std::strtol(lenStr.c_str(), &endPtr, 10);
	if (endPtr == lenStr.c_str() || *endPtr != '\0' || contentLength < 0 ||
		contentLength > static_cast<long>(maxBodySize)) {
		return false;
	}

	if (buffer.size() - bodyStart < static_cast<size_t>(contentLength)) {
		return false;
	}

	_body.append(buffer, bodyStart, static_cast<size_t>(contentLength));
	_complete = true;
	buffer.erase(0, bodyStart + static_cast<size_t>(contentLength));
	return true;
}

bool HttpRequest::isComplete() const { return this->_complete; }

const std::string &HttpRequest::getMethod() const { return this->_method; }

const std::string &HttpRequest::getPath() const { return this->_path; }

const std::string &HttpRequest::getVersion() const { return this->_version; }

const std::string &HttpRequest::getBody() const { return this->_body; }

const std::string &HttpRequest::getHeader(const std::string &header) const {
	try {
		return this->_headers.at(header);
	} catch (const std::out_of_range &) {
		static const std::string empty;
		return empty;
	}
}

// debug
void HttpRequest::printData() {
	std::map<std::string, std::string>::iterator it;

	std::cout << "======================================================\n";
	std::cout << "isComplete: " << _complete << "\n";
	std::cout << "method: " << _method << "\n";
	std::cout << "path: " << _path << "\n";
	std::cout << "query: " << "\n";
	for (it = _query.begin(); it != _query.end(); ++it) {
		std::cout << "\t" << it->first << ": " << it->second << std::endl;
	}
	std::cout << "version: " << _version << "\n";
	std::cout << "header: " << "\n";
	for (it = _headers.begin(); it != _headers.end(); ++it) {
		std::cout << "\t" << it->first << ": " << it->second << std::endl;
	}
	std::cout << "body: " << _body << "\n";
	std::cout << "======================================================\n";
}
