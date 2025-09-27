#ifndef INC_42_WEBSERV_COLORS_HPP
#define INC_42_WEBSERV_COLORS_HPP

#include <string>

namespace  Colors {
	const std::string RESET = "\033[0m";
	const std::string BOLD = "\033[1m";
	const std::string RED = "\033[38;2;255;0;0m";
	const std::string GREEN = "\033[38;2;0;255;0m";
	const std::string BLUE = "\033[38;2;0;0;255m";
	const std::string CYAN = "\033[38;2;0;255;255m";
	const std::string MAGENTA = "\033[38;2;255;0;255m";
	const std::string YELLOW = "\033[38;2;255;255;0m";
} // namespace Colors

#endif //INC_42_WEBSERV_COLORS_HPP
