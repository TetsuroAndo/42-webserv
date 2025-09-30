#pragma once

#include <map>
#include <ostream>
#include <set>
#include <string>
#include <vector>

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
	std::string path;
	std::string root;
	std::set<std::string> allowedMethods;
	bool autoindex;
	std::string indexFile;
	std::string errorFile;
	std::string uploadStore;
	std::map<std::string, std::string> cgiConf;

	Location() : autoindex(false) {}
};

struct AccessLog {
	bool isDisable;
	std::string sink;
	std::string filename;
	std::string logDir;
	std::string format;
	size_t maxFileSize;
	size_t maxBackupFiles;
};

struct ErrorLog {
	bool isDisable;
	std::string sink;
	std::string filename;
	std::string logDir;
	std::string format;
	std::string level;
	std::string filterMode;
	size_t maxFileSize;
	size_t maxBackupFiles;
};

class Config {
private:
	std::vector<Listen> _listens;
	std::map<std::string, Redirect> _redirects;
	std::map<std::string, Location> _locations;
	std::vector<AccessLog> _accessLogs;
	std::vector<ErrorLog> _errorLogs;
	unsigned int _maxRequestBodySize;
	unsigned int _timeoutSec;
	unsigned int _maxEvents;

	void setRoot(const std::string &root, const std::string &locationKey = "/");
	void setAutoindex(bool autoindex, const std::string &locationKey = "/");
	void setIndexFile(const std::string &indexFile,
					  const std::string &locationKey = "/");
	void setErrorFile(const std::string &errorFile,
					  const std::string &locationKey = "/");
	void setUploadStore(const std::string &uploadStore,
						const std::string &locationKey = "/");

	void setCgiConf(const std::string &extension,
					const std::string &interpreterPath,
					const std::string &locationKey = "/");

	void setIsAllowGet(bool allow, const std::string &locationKey = "/");
	void setIsAllowHead(bool allow, const std::string &locationKey = "/");
	void setIsAllowPost(bool allow, const std::string &locationKey = "/");
	void setIsAllowDelete(bool allow, const std::string &locationKey = "/");
	void setAllowedMethods(const std::string &methods,
						   const std::string &locationKey = "/");
	void setAllowedMethods(const std::set<std::string> &methods,
						   const std::string &locationKey = "/");

	void setListens(const std::vector<Listen> &lists);
	void setRedirects(const std::map<std::string, Redirect> &redirects);
	void setRedirect(const Redirect &redirect,
					 const std::string &redirectKey = "/");
	void setLocations(const std::map<std::string, Location> &locations);
	void setLocation(const Location &location,
					 const std::string &locationKey = "/");

	void setMaxRequestBodySize(unsigned int size);
	void setTimeoutSec(unsigned int sec);
	void setMaxEvents(unsigned int maxEvents);

	void initDefaults();
	void setup(const std::string &configFile);

	void parseListens(const Node *node);
	void parseRedirects(Node *node);
	void parseLocations(Node *node);
	void parseAccessLogs(Node *node);
	void parseErrorLogs(Node *node);

public:
	Config();
	Config(const std::string &configFile);
	Config(const Config &other);
	Config &operator=(const Config &other);
	~Config();

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
