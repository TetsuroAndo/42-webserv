#pragma once

#include <ostream>

class Config {
public:
    Config();
    ~Config();

    Config(const Config&);
    Config& operator=(const Config&);

	int getPort()const;

	friend std::ostream& operator<<(std::ostream& os, const Config& config);
private:
	int port;
	void setPort(int port);

};
