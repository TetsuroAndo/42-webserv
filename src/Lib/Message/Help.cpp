#include "../../Config/Info/App.hpp"
#include <iostream>

/**
 * @brief プログラムの基本的な使用法を表示する
 * @param progName プログラム名 (argv[0])
 *
 * エラーメッセージとして標準エラー出力(std::cerr)に表示するのが一般的です。
 */
void printUsage(const char *progName) {
	std::cerr << "Usage: " << progName << " [Configuration file]" << std::endl;
}

/**
 * @brief プログラムのバージョン情報を表示する
 * @param progName プログラム名 (argv[0])
 * @param version バージョン文字列
 */
void printVersion(const char *version) {
	std::cout << SOFTWARE_NAME << " version " << version << std::endl;
}

/**
 * @brief 詳細なヘルプ情報を表示する
 * @param progName プログラム名 (argv[0])
 *
 * 通常の出力として標準出力(std::cout)に表示します。
 */
void printHelp(const char *progName) {
	std::cout << SOFTWARE_NAME << " - A simple HTTP Server program.\n";
	std::cout << std::endl;
	printUsage(progName);
	std::cout << std::endl;
	std::cout << "Options:\n";
	std::cout << "  -h, --help       Show this help message and exit.\n";
	std::cout << "  -v, --version    Show program's version number and exit."
			  << std::endl;
}
