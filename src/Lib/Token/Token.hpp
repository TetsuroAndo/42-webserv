#include <iostream>
#include <string>
#include <random>
#include <vector>

class Token {
public:
	Token();
	~Token();

	std::string genToken(size_t length = 32);
private:
	std::mt19937 _generator;
};
