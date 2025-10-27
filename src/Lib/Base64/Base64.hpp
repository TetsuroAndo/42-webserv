#pragma once

#include <string>

namespace Base64 {
std::string encode(const std::string &input);
std::string decode(const std::string &input);
} // namespace Base64
