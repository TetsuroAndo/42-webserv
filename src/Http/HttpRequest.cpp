#include <cstdlib>
#include <iostream>
#include <sstream>

#include "../Lib/URI/uri.hpp"
#include "HttpRequest.hpp"
#include "HttpRequestHelper.hpp"

HttpRequest::HttpRequest() : _parseStatus(PARSE_INCOMPLETE), _error(NONE) {}

HttpRequest::~HttpRequest() {}

ParseStatus HttpRequest::parse(std::string &buffer) {
	if (_parseStatus == PARSE_COMPLETE) {
		return _parseStatus;
	}

	size_t headerEnd = buffer.find("\r\n\r\n");
	if (headerEnd == std::string::npos) {
		return _parseStatus;
	}
	if (headerEnd > maxHeaderSize) {
		setError(PARSE_ERROR_LARGE_HEADER);
		return _parseStatus;
	}
	std::string headerPart = buffer.substr(0, headerEnd);
	std::istringstream headerStream(headerPart);
	std::string requestLine;

	if (!std::getline(headerStream, requestLine)) {
		return _parseStatus;
	}
	if (!HttpRequest::parseRequestLine(requestLine)) {
		return _parseStatus;
	}
	if (!HttpRequest::parseHeaders(headerStream)) {
		return _parseStatus;
	}
	if (!HttpRequest::parseBody(buffer, headerEnd + 4)) {
		return _parseStatus;
	}
	return _parseStatus;
}

bool HttpRequest::parseRequestLine(std::string &requestLine) {
	HttpRequestHelper::trimCR(requestLine);
	if (requestLine.empty()) {
		setError(PARSE_ERROR_INVALID_REQUEST);
		return false;
	}
	if (!splitRequestLine(requestLine)) {
		return false;
	}
	splitPathAndQuery();
	parseQueryString();
	_parseStatus = PARSE_INCOMPLETE;
	return true;
}

bool HttpRequest::splitRequestLine(const std::string &requestLine) {
	std::istringstream iss(requestLine);
	if (!(iss >> _method >> _path >> _version)) {
		setError(PARSE_ERROR_INVALID_REQUEST);
		return false;
	}
	std::string extra;
	if (iss >> extra) {
		setError(PARSE_ERROR_INVALID_REQUEST);
		return false;
	}
	if (_method != "GET" && _method != "POST" && _method != "DELETE") {
		setError(PARSE_ERROR_HTTP_METHOD);
		return false;
	}
	if (_version != "HTTP/1.0" && _version != "HTTP/1.1") {
		setError(PARSE_ERROR_HTTP_VERSION);
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

bool HttpRequest::parseHeaders(std::istringstream &headerStream) {
	size_t parsedHeaderBytes = 0;
	std::string line;
	while (std::getline(headerStream, line)) {
		parsedHeaderBytes += line.size() + 2;
		if (parsedHeaderBytes > maxHeaderSize) {
			setError(PARSE_ERROR_LARGE_HEADER);
			return false;
		}

		HttpRequestHelper::trimCR(line);

		size_t pos = line.find(":");
		if (pos != std::string::npos) {
			std::string key = line.substr(0, pos);
			HttpRequestHelper::trimSpaces(key, " \t");
			HttpRequestHelper::toLower(key);
			std::string value = line.substr(pos + 1);
			HttpRequestHelper::trimSpaces(value, " \t");
			if (_headers.find(key) != _headers.end()) {
				setError(PARSE_ERROR_INVALID_REQUEST);
				return false;
			}
			_headers[key] = value;
		} else {
			setError(PARSE_ERROR_INVALID_REQUEST);
			return false;
		}
	}
	return true;
}

bool HttpRequest::parseBody(std::string &buffer, size_t bodyStart) {
	if (_headers.find("transfer-encoding") != _headers.end() &&
		_headers.find("content-length") != _headers.end()) {
		setError(PARSE_ERROR_INVALID_REQUEST);
		return false;
	}
	if (_headers.find("transfer-encoding") != _headers.end() &&
		_headers["transfer-encoding"].find("chunked") != std::string::npos) {
		return parseChunkedBody(buffer, bodyStart);
	}
	if (_headers.find("content-length") != _headers.end()) {
		return parseContentLengthBody(buffer, bodyStart);
	}
	_body = "";
	_parseStatus = PARSE_COMPLETE;
	buffer.erase(0, bodyStart);
	return true;
}

bool HttpRequest::parseChunkedBody(std::string &buffer, size_t bodyStart) {
	size_t erasePos = bodyStart;
	size_t pos = bodyStart;

	while (true) {
		size_t chunkSize;
		size_t nextPos;
		int chunkStatus =
			HttpRequestHelper::getChunkSize(buffer, pos, chunkSize, nextPos);
		if (chunkStatus == HttpRequestHelper::CHUNK_ERROR) {
			setError(PARSE_ERROR_INVALID_REQUEST);
			return false;
		}
		if (chunkStatus == HttpRequestHelper::CHUNK_INCOMPLETE) {
			return false;
		}
		pos = nextPos;
		if (chunkSize == 0) {
			erasePos = std::min(HttpRequestHelper::handleLastChunk(buffer, pos),
								buffer.size());
			break;
		}

		int readStatus = HttpRequestHelper::readChunkData(
			buffer, pos, _body, chunkSize, maxBodySize);
		if (readStatus == HttpRequestHelper::CHUNK_ERROR) {
			setError(PARSE_ERROR_INVALID_REQUEST);
			return false;
		}
		if (readStatus == HttpRequestHelper::CHUNK_INCOMPLETE) {
			return false;
		}
		erasePos = pos;
	}

	if (erasePos > 0)
		buffer.erase(0, erasePos);
	_parseStatus = PARSE_COMPLETE;
	return true;
}

bool HttpRequest::parseContentLengthBody(std::string &buffer,
										 size_t bodyStart) {
	std::string lenStr = _headers["content-length"];
	HttpRequestHelper::trimSpaces(lenStr, " \t\n\r\f\v");

	char *endPtr = NULL;
	size_t contentLength = std::strtoul(lenStr.c_str(), &endPtr, 10);
	if (endPtr == lenStr.c_str() || *endPtr != '\0' ||
		contentLength > maxBodySize) {
		setError(PARSE_ERROR_LARGE_REQUEST);
		return false;
	}

	if (buffer.size() - bodyStart < contentLength) {
		return false;
	}

	_body.append(buffer, bodyStart, contentLength);
	_parseStatus = PARSE_COMPLETE;
	buffer.erase(0, bodyStart + contentLength);
	return true;
}

void HttpRequest::setError(ParseErrorStatus status) {
	_parseStatus = PARSE_ERROR;
	_error = status;
}

bool HttpRequest::isComplete() const { return _parseStatus == PARSE_COMPLETE; }

int HttpRequest::getError() const { return static_cast<int>(_error); }

const std::string &HttpRequest::getMethod() const { return this->_method; }

const std::string &HttpRequest::getPath() const { return this->_path; }

const std::string &HttpRequest::getVersion() const { return this->_version; }

const std::string &HttpRequest::getBody() const { return this->_body; }

const std::string &HttpRequest::getHeader(const std::string &header) const {
	std::string lowerHeader = header;
	HttpRequestHelper::toLower(lowerHeader);
	std::map<std::string, std::string>::const_iterator it =
		this->_headers.find(lowerHeader);
	if (it != this->_headers.end()) {
		return it->second;
	}
	static const std::string empty;
	return empty;
}

// debug
void HttpRequest::printData() {
	std::map<std::string, std::string>::iterator it;

	std::cout << "======================================================\n";
	if (_parseStatus == PARSE_COMPLETE) {
		std::cout << "parseStatus: " << "PARSE_COMPLETE" << "\n";
	} else if (_parseStatus == PARSE_INCOMPLETE) {
		std::cout << "parseStatus: " << "PARSE_INCOMPLETE" << "\n";
	} else if (_parseStatus == PARSE_ERROR) {
		std::cout << "parseStatus: " << "PARSE_ERROR" << "\n";
		std::cout << "parseErrorStatus: " << getError() << "\n";
	}
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
	std::cout << "======================================================"
			  << std::endl;
}
