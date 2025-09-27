#pragma once

#include <ostream>
#include <string>
#include <vector>
#include <set>
#include <map>

class Node;

struct Listen {
	std::string interface;
	int port;
};

struct Redirect {
	std::string fromPath;
	std::string toUrl;
	int code;
};

struct Location {
	std::string path;							// e.g., "/" or "/cgi-bin"
	std::string root;							// The root directory for this location
	std::set<std::string> allowedMethods;		// "GET", "HEAD", "POST", "DELETE" を保持
	bool autoindex;								// ディレクトリリスティングの on/off
	std::string indexFile;						// 表示するファイル名
	std::string errorFile;						// エラーページ
	std::string uploadStore;					// アップロードファイルの保存先ディレクトリ
	std::map<std::string, std::string> cgiConf;	// CGI設定 key: 拡張子 (e.g., ".php"), value: インタプリタのパス (e.g., "/usr/bin/php-cgi")

	Location() : autoindex(false) {}
};

class Config {
private:
	std::vector<Listen> _listens;
	std::map<std::string, Redirect> _redirects;	// key: from_path
	std::map<std::string, Location> _locations;	// key: path
	unsigned int _maxRequestBodySize;
	unsigned int _timeoutSec;
	unsigned int _maxEvents;

	// Set Default Values
	void setRoot(const std::string &root, const std::string &locationKey = "/");
	void setAutoindex(bool autoindex, const std::string &locationKey = "/");
	void setIndexFile(const std::string &indexFile, const std::string &locationKey = "/");
	void setErrorFile(const std::string &errorFile, const std::string &locationKey = "/");
	void setUploadStore(const std::string &uploadStore, const std::string &locationKey = "/");

	// Set Default CGI
	void setCgiConf(const std::string &extension, const std::string &interpreterPath, const std::string &locationKey = "/");

	// Set Default Methods
	void setIsAllowGet(bool allow, const std::string &locationKey = "/");
	void setIsAllowHead(bool allow, const std::string &locationKey = "/");
	void setIsAllowPost(bool allow, const std::string &locationKey = "/");
	void setIsAllowDelete(bool allow, const std::string &locationKey = "/");
	void setAllowedMethods(const std::string &methods, const std::string &locationKey = "/");
	void setAllowedMethods(const std::set<std::string> &methods, const std::string &locationKey = "/");

	void setListens(const std::vector<Listen> &lists);
	void setRedirects(const std::map<std::string, Redirect> &redirects);
	void setRedirect(const Redirect &redirect, const std::string &redirectKey = "/");
	void setLocations(const std::map<std::string, Location> &locations);
	void setLocation(const Location &location, const std::string &locationKey = "/");

	void setMaxRequestBodySize(unsigned int size);
	void setTimeoutSec(unsigned int sec);
	void setMaxEvents(unsigned int maxEvents);

	void setup(const std::string& configFile);
	void setup_hardcoded();
	void parseListens(Node* node);
    void parseRedirects(Node* node);
    void parseLocations(Node* node);

public:
	Config();
	Config(const std::string &configFile);
	Config(const Config &other);
	Config &operator=(const Config &other);
	~Config();

	// Getters
	const std::vector<Listen> &getListens() const;
	const std::map<std::string, Redirect> &getRedirects() const;
	const Redirect &getRedirect(const std::string &path) const;
	const std::map<std::string, Location> &getLocations() const;
	const Location &getLocation(const std::string &path) const;

	unsigned int getMaxRequestBodySize() const;
	unsigned int getTimeoutSec() const;
	unsigned int getMaxEvents() const;

	friend std::ostream &operator<<(std::ostream &os, const Config &config);
};
