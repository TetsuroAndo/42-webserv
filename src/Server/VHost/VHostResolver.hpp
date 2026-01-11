#pragma once

#include "../Listen/ListenKey.hpp"
#include "VirtualHost.hpp"
#include <string>
#include <vector>

class VHostResolver {
public:
	static VirtualHost *find(std::vector< VirtualHost > &vhosts,
							 const ListenKey &listenKey,
							 const std::string &hostHeader);

private:
	VHostResolver();
	~VHostResolver();
	VHostResolver(const VHostResolver &);
	VHostResolver &operator=(const VHostResolver &);
};
