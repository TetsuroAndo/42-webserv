#pragma once
#include <map>
#include <string>
#include <vector>

class MyYAML
{
public:
	MyYAML(const std::string &filepath);
    ~MyYAML();

	void debugAllKeyAndValue();

	class FileNotFound : public std::exception {
		const char *what () const throw() { return "File not found"; }
	};

	class InvalidFormat : public std::exception {
		const char *what () const throw() { return "Invalid format"; }
	};

private:
	std::map<std::string, std::vector<std::string> > myYamlData;
	static std::string readFileAll(const std::string &filepath);
	void parseYaml(std::string buf);


    MyYAML();

    MyYAML(const MyYAML&);
    MyYAML& operator=(const MyYAML&);
};