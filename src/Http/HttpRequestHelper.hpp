#ifndef HTTP_REQUEST_HELPER_HPP
#define HTTP_REQUEST_HELPER_HPP

#include <cstddef>
#include <string>

namespace HttpRequestHelper {

void trimCR(std::string &line);
void trimSpaces(std::string &s, const std::string &spaces);
void toLower(std::string &str);

enum ChunkStatus { CHUNK_INCOMPLETE, CHUNK_COMPLETE, CHUNK_ERROR };

ChunkStatus getChunkSize(const std::string &buffer, size_t pos,
						 size_t &chunkSize, size_t &nextPos);

ChunkStatus readChunkData(const std::string &buffer, size_t &pos,
						  std::string &body, size_t chunkSize,
						  size_t maxBodySize);

size_t handleLastChunk(const std::string &buffer, size_t pos);

} // namespace HttpRequestHelper

#endif
