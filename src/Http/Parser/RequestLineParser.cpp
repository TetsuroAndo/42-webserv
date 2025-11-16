#include "RequestLineParser.hpp"
#include "../Core/HttpStatus.hpp"
#include "../URI/URI.hpp"
#include <cstring>

RequestLineParser::RequestLineParser() {}

RequestLineParser::~RequestLineParser() {}

bool RequestLineParser::parse(HttpRequest &request, const std::string &line,
							  int &errorCode) {
	const size_t methodEnd = line.find(' ');
	if (methodEnd == std::string::npos) {
		errorCode = HttpStatus::BAD_REQUEST;
		return false;
	}

	const size_t uriEnd = line.find(' ', methodEnd + 1);
	if (uriEnd == std::string::npos) {
		errorCode = HttpStatus::BAD_REQUEST;
		return false;
	}

	const size_t methodLen = methodEnd;
	const std::string method(line.c_str(), methodLen);
	request.setMethod(method);

	const char *uriStart = line.c_str() + methodEnd + 1;
	const size_t uriLen = uriEnd - (methodEnd + 1);

	const std::string version(line.c_str() + uriEnd + 1,
							  line.length() - (uriEnd + 1));
	request.setVersion(version);

	if (version != "HTTP/1.1" && version != "HTTP/1.0") {
		errorCode = HttpStatus::VERSION_NOT_SUPPORTED;
		return false;
	}

	// 3. URIをパスとクエリに分割
	const char *queryStartPtr =
		static_cast< const char * >(std::memchr(uriStart, '?', uriLen));
	size_t pathLen;
	if (queryStartPtr) {
		pathLen = queryStartPtr - uriStart;
		const char *queryStart = queryStartPtr + 1;
		const size_t queryLen = uriLen - pathLen - 1;

		// 4. クエリをキーと値のペアに分割
		size_t queryOffset = 0;
		while (queryOffset < queryLen) {
			size_t pairEndOffset;
			const char *ampPtr = static_cast< const char * >(std::memchr(
				queryStart + queryOffset, '&', queryLen - queryOffset));
			if (ampPtr) {
				pairEndOffset = ampPtr - queryStart;
			} else {
				pairEndOffset = queryLen;
			}

			const char *pairStart = queryStart + queryOffset;
			const size_t pairLen = pairEndOffset - queryOffset;

			const char *eqPtr = static_cast< const char * >(
				std::memchr(pairStart, '=', pairLen));
			std::string key, value;
			if (eqPtr) {
				key = URI::decodeURIComponent(
					std::string(pairStart, eqPtr - pairStart));
				value = URI::decodeURIComponent(
					std::string(eqPtr + 1, pairStart + pairLen - (eqPtr + 1)));
			} else {
				key = URI::decodeURIComponent(std::string(pairStart, pairLen));
				value = "";
			}
			request.addQuery(key, value);

			queryOffset = pairEndOffset + 1;
		}
	} else {
		pathLen = uriLen;
	}

	request.setPath(std::string(uriStart, pathLen));
	return true;
}
