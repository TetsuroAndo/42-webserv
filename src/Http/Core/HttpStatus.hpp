#pragma once

#include <string>
#include <map>

namespace HttpStatus {

	/**
	 * @brief HTTPステータスコードに対応する理由フレーズを返します。
	 *
	 * @param code HTTPステータスコード (e.g., 200, 404).
	 * @return const std::string& 理由フレーズ (e.g., "OK", "Not Found").
	 * 見つからない場合は "Internal Server Error" のフレーズを返します。
	 */
	const std::string &getReason(int code);

} // namespace HttpStatus
