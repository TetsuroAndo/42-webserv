#pragma once

#include "../../Config/Config.hpp"
#include "../../Middleware/Core/MiddlewareProcessor.hpp"

struct VirtualHost {
	Config config;
	MiddlewareProcessor mainProcessor;

	VirtualHost(const Config &conf) : config(conf), mainProcessor() {}
};
