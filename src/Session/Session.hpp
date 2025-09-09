#pragma once

#include <map>
#include <string>

class SessionManager{
public:
	SessionManager();
	~SessionManager();


private:
	std::map<int, Client*> _clients;
};
