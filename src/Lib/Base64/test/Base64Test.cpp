// Base64 unit test for src/Lib/Base64
#include "Base64.hpp"
#include <cassert>
#include <iostream>
#include <string>
#include <vector>

using namespace Base64;

struct TestCase {
	std::string input;
	std::string expectedEncoded;
};

namespace {
TestCase make(const char *in, const char *exp) {
	TestCase t;
	t.input = in;
	t.expectedEncoded = exp;
	return t;
}
} // namespace

int main() {
	std::vector< TestCase > tests;

	// --- 基本テスト ---
	tests.push_back(make("", ""));
	tests.push_back(make("f", "Zg=="));
	tests.push_back(make("fo", "Zm8="));
	tests.push_back(make("foo", "Zm9v"));
	tests.push_back(make("foob", "Zm9vYg=="));
	tests.push_back(make("fooba", "Zm9vYmE="));
	tests.push_back(make("foobar", "Zm9vYmFy"));

	// --- 追加テスト: アルファベット・数字・記号混合 ---
	tests.push_back(make("Man", "TWFu"));
	tests.push_back(make("Hello, World!", "SGVsbG8sIFdvcmxkIQ=="));
	tests.push_back(make("Base64 encode/decode test.",
						 "QmFzZTY0IGVuY29kZS9kZWNvZGUgdGVzdC4="));

	// --- 全バイト (0x00〜0xFF) の動作確認 ---
	std::string allBytes;
	for (int i = 0; i < 256; ++i)
		allBytes.push_back(static_cast< char >(i));
	std::string encodedAll = encode(allBytes);
	std::string decodedAll = decode(encodedAll);
	assert(decodedAll == allBytes);
	std::cout << "✅ All-bytes test passed (0x00〜0xFF)\n";

	// --- 各テストケース ---
	for (std::size_t i = 0; i < tests.size(); ++i) {
		const std::string &input = tests[i].input;
		const std::string encoded = encode(input);
		const std::string decoded = decode(encoded);

		if (encoded != tests[i].expectedEncoded) {
			std::cerr << "❌ Encode mismatch (" << i << ")\n"
					  << " input   = \"" << input << "\"\n"
					  << " expected= \"" << tests[i].expectedEncoded << "\"\n"
					  << " got     = \"" << encoded << "\"\n";
			return 1;
		}

		if (decoded != input) {
			std::cerr << "❌ Decode mismatch (" << i << ")\n"
					  << " encoded = \"" << encoded << "\"\n"
					  << " expected= \"" << input << "\"\n"
					  << " got     = \"" << decoded << "\"\n";
			return 1;
		}

		std::cout << "✅ Test " << i << " passed: \"" << input << "\" → "
				  << encoded << std::endl;
	}

	std::cout << "\n🎉 All Base64 tests passed successfully!\n";
	return 0;
}
