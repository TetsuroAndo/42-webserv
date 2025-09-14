#include "HttpRequestHelper.hpp"

#include <algorithm>
#include <cctype>
#include <cstdlib>

namespace HttpRequestHelper {

void trimCR(std::string &line) {
	if (!line.empty() && line[line.size() - 1] == '\r') {
		line.erase(line.size() - 1);
	}
}

void trimSpaces(std::string &s, const std::string &spaces) {
	size_t start = s.find_first_not_of(spaces);
	size_t end = s.find_last_not_of(spaces);
	if (start == std::string::npos) {
		s.clear();
	} else {
		s = s.substr(start, end - start + 1);
	}
}

void toLower(std::string &str) {
	for (std::string::size_type i = 0; i < str.size(); ++i) {
		str[i] =
			static_cast<char>(std::tolower(static_cast<unsigned char>(str[i])));
	}
}

ChunkStatus getChunkSize(const std::string &buffer, size_t pos,
						 size_t &chunkSize, size_t &nextPos) {
	size_t crlf = buffer.find("\r\n", pos);
	if (crlf == std::string::npos) {
		return CHUNK_INCOMPLETE;
	}

	std::string chunkSizeStr = buffer.substr(pos, crlf - pos);
	size_t semiPos = chunkSizeStr.find(";");
	if (semiPos != std::string::npos)
		chunkSizeStr = chunkSizeStr.substr(0, semiPos);

	char *endptr = NULL;
	chunkSize = std::strtoul(chunkSizeStr.c_str(), &endptr, 16);
	if (endptr == chunkSizeStr.c_str() || *endptr != '\0') {
		return CHUNK_ERROR;
	}

	nextPos = crlf + 2;
	return CHUNK_COMPLETE;
}

ChunkStatus readChunkData(const std::string &buffer, size_t &pos,
						  std::string &body, size_t chunkSize,
						  size_t maxBodySize) {
	if (pos + chunkSize + 2 > buffer.size()) {
		return CHUNK_INCOMPLETE;
	}
	if (body.size() + chunkSize > maxBodySize) {
		return CHUNK_ERROR;
	}

	body.append(buffer, pos, chunkSize);
	pos += chunkSize;

	if (buffer.compare(pos, 2, "\r\n") != 0) {
		return CHUNK_ERROR;
	}
	pos += 2;
	return CHUNK_COMPLETE;
}

size_t handleLastChunk(const std::string &buffer, size_t pos) {
	size_t trailerEnd = buffer.find("\r\n\r\n", pos);

	if (buffer.compare(pos, 2, "\r\n") == 0) {
		pos += 2;
	} else if (trailerEnd != std::string::npos) {
		pos = trailerEnd + 4;
	}
	return std::min(pos, buffer.size());
}

} // namespace HttpRequestHelper
