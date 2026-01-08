#pragma once

#include "../../Config/Config.hpp"

namespace ServerBootstrap {

/// @brief
std::string listenToString(const Listen &listen);
/// @brief
const Config &selectCgiConfig(const std::vector< Config > &configs);
/// @brief
size_t resolveMaxEvents(const std::vector< Config > &configs);
/// @brief
size_t resolveMaxSessionTimeout(const std::vector< Config > &configs);
/// @brief
void validateListenCompatibility(const std::vector< Config > &configs);

} // namespace ServerBootstrap
