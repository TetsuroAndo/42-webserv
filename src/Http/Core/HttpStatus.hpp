#pragma once

#include <string>

namespace HttpStatus {

	enum {
		// Informational
		CONTINUE = 100,
		// Success
		OK = 200,
		CREATED = 201,
		ACCEPTED = 202,
		NO_CONTENT = 204,
		// Redirection
		MOVED_PERMANENTLY = 301,
		FOUND = 302,
		SEE_OTHER = 303,
		// Client Error
		BAD_REQUEST = 400,
		UNAUTHORIZED = 401,
		FORBIDDEN = 403,
		NOT_FOUND = 404,
		METHOD_NOT_ALLOWED = 405,
		REQUEST_TIMEOUT = 408,
		PAYLOAD_TOO_LARGE = 413,
		URI_TOO_LONG = 414,
		UNSUPPORTED_MEDIA_TYPE = 415,
		REQUEST_HEADER_FIELDS_TOO_LARGE = 431,
		/// Server Error
		INTERNAL_SERVER_ERROR = 500,
		NOT_IMPLEMENTED = 501,
		BAD_GATEWAY = 502,
		SERVICE_UNAVAILABLE = 503,
		GATEWAY_TIMEOUT = 504,
		VERSION_NOT_SUPPORTED = 505
	};

	/**
	 * @brief HTTPステータスコードに対応する理由フレーズを返します。
	 *
	 * @param code HTTPステータスコード (e.g., 200, 404).
	 * @return const std::string& 理由フレーズ (e.g., "OK", "Not Found").
	 * 見つからない場合は "Internal Server Error" のフレーズを返します。
	 */
	const std::string &getReason(int code);

} // namespace HttpStatus
