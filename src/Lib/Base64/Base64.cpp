#include "Base64.hpp"
#include <bitset>

namespace {
int decodeChar(char c) {
	if ('A' <= c && c <= 'Z')
		return c - 'A';
	if ('a' <= c && c <= 'z')
		return c - 'a' + 26;
	if ('0' <= c && c <= '9')
		return c - '0' + 52;
	if (c == '+')
		return 62;
	if (c == '/')
		return 63;
	return -1;
}
} // namespace

namespace Base64 {
static const std::string table =
	"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

std::string encode(const std::string &input) {
	std::string output;
	int i = 0;
	while (i < input.size()) {
		unsigned char buf3[3];
		unsigned char buf4[4];
		int len = 0;
		int readBytes = 0;
		for (len = 0; len < 3; len++) {
			if (i < input.size()) {
				buf3[len] = input[i++];
				readBytes++;
			} else {
				buf3[len] = 0;
				break;
			}
		}
		buf4[0] = (buf3[0] & 0xfc) >> 2; // [0]1~6
		buf4[1] =
			(buf3[0] & 0x03) << 4 | (buf3[1] & 0xf0) >> 4; // [0]7~8,[1]1~4
		buf4[2] =
			(buf3[1] & 0x0f) << 2 | (buf3[2] & 0xc0) >> 6; // [1]4~8, [2]1~2
		buf4[3] = buf3[2] & 0x3f;						   //[2]3~8

		for (int j = 0; j < 4; ++j) {
			if (j < readBytes + 1)
				output.push_back(table[buf4[j]]);
			else
				output.push_back('=');
		}
	}
	return output;
}

std::string decode(const std::string &input) {
	std::string output;
	unsigned int buf3[3];
	unsigned int buf4[4];

	int count = 0;
	for (int i = 0; i < input.size(); i++) {
		if (input[i] == '=')
			break;
		const int val = decodeChar(input[i]);
		if (val < 0) {
			continue;
		}
		buf4[count] = val;
		count++;
		if (count == 4) {
			buf3[0] = buf4[0] << 2 | (buf4[1] & 0x30) >> 4; // [0]1~6 + [1]1~2
			buf3[1] = (buf4[1] & 0x0f) << 4 |
					  (buf4[2] & 0x3c) >> 2;		   // [1]3~6 + [2]1~4
			buf3[2] = (buf4[2] & 0x03) << 6 | buf4[3]; // [2]5~6 + [3]1~6
			for (int j = 0; j < 3; j++) {
				output.push_back(static_cast< char >(buf3[j]));
			}
			count = 0;
		}
	}
	if (count > 1) {
		output.push_back(
			static_cast< char >(buf4[0] << 2 | (buf4[1] & 0x30) >> 4));
	}
	if (count > 2) {
		output.push_back(
			static_cast< char >((buf4[1] & 0x0f) << 4 | (buf4[2] & 0x3c) >> 2));
	}
	return output;
}
} // namespace Base64
