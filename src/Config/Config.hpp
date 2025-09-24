#pragma once

#include <ostream>
#include <string>
#include <vector>
#include <set>

struct Listen {
	std::string interface;
	int port;
};

struct Location {
	std::string path;							// e.g., "/" or "/cgi-bin"
	std::string root;							// The root directory for this location
	std::set<std::string> allowedMethods;		// "GET", "HEAD", "POST", "DELETE" を保持
	bool autoindex = true;						// ディレクトリリスティングの on/off
	std::string redirectUrl;					// リダイレクト先のURL
	std::string indexFile;						// デフォルトで表示するファイル名
	std::string errorFile;						// デフォルトエラーページ
	std::string uploadStore;					// アップロードファイルの保存先ディレクトリ
	std::map<std::string, std::string> cgiConf;	// CGI設定 key: 拡張子 (e.g., ".php"), value: インタプリタのパス (e.g., "/usr/bin/php-cgi")
};

class Config {
private:
	std::vector<Listen> _listens;
	std::vector<Location> _locations;
	unsigned int _maxRequestBodySize;
	unsigned int _timeoutSec;
	unsigned int _maxEvents;

	void setListens(const std::vector<Listen> &lists);
	void setLocations(const std::vector<Location> &locations);
	void setMaxRequestBodySize(unsigned int size);
	void setTimeoutSec(unsigned int sec);
	void setMaxEvents(unsigned int maxEvents);

public:
	Config();
	Config(std::string configFile);
	~Config();
	Config(const Config &other);
	Config &operator=(const Config &other);

	void setup(const std::string& configFile = "");

	const std::vector<Listen> &getListens() const;
	const std::vector<Location> &getLocations() const;
	unsigned int getMaxRequestBodySize() const;
	unsigned int getTimeoutSec() const;
	unsigned int getMaxEvents() const;

	friend std::ostream &operator<<(std::ostream &os, const Config &config);
};
