#include "../Token.hpp"
#include <iomanip>
#include <iostream>
#include <sstream>
#include <sys/time.h>

int main() {
	const size_t NUM_TOKENS = 1000000;
	const size_t TOKEN_LENGTH = 32;

	std::cout << "Generating " << NUM_TOKENS
			  << " tokens (length: " << TOKEN_LENGTH << ")..." << std::endl;

	// 進捗メッセージを蓄積するストリーム
	std::ostringstream progress_stream;

	// 開始時間を記録
	struct timeval start, end;
	gettimeofday(&start, NULL);

	// 100万トークンを生成
	for (size_t i = 0; i < NUM_TOKENS; ++i) {
		std::string token = Token::getInstance().genToken(TOKEN_LENGTH);
		// 進捗表示（10万ごと）- 文字列ストリームに蓄積
		if ((i + 1) % 100000 == 0) {
			progress_stream << "Generated " << (i + 1) << " tokens..." << '\n';
		}
	}

	// 終了時間を記録
	gettimeofday(&end, NULL);

	// 経過時間を計算（秒とマイクロ秒）
	long seconds = end.tv_sec - start.tv_sec;
	long microseconds = end.tv_usec - start.tv_usec;
	if (microseconds < 0) {
		seconds--;
		microseconds += 1000000;
	}

	double elapsed_seconds = seconds + microseconds / 1000000.0;
	double tokens_per_second = NUM_TOKENS / elapsed_seconds;

	// 進捗メッセージをまとめて出力
	std::cout << progress_stream.str();

	// 結果を表示
	std::cout << std::fixed << std::setprecision(3);
	std::cout << "\n=== Results ===" << std::endl;
	std::cout << "Total tokens: " << NUM_TOKENS << std::endl;
	std::cout << "Elapsed time: " << elapsed_seconds << " seconds" << std::endl;
	std::cout << "Tokens per second: " << tokens_per_second << std::endl;
	std::cout << "Average time per token: "
			  << (elapsed_seconds / NUM_TOKENS * 1000000) << " microseconds"
			  << std::endl;

	return 0;
}
