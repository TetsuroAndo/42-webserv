#pragma once
#include <ostream>
#include <string>
#include <vector>

struct Listen {
	std::string interface;
	int port;
};

struct Location {
	std::string path;
	std::string root;
};

class Config {
private:
	std::vector<Listen> _listens;
	std::string _defaultErrorPage;
	unsigned int _maxRequestBodySize;
	bool _isAllowGet;
	bool _isAllowPost;
	bool _isAllowHead;
	bool _isAllowDelete;
	std::string _redirect;
	std::vector<Location> _locations;
	bool _isShowDirectoryListPage;
	std::string _whenRequestedDirectory;
	std::string _saveFileDirectory;
	unsigned int _timeoutSec;
	unsigned int _maxEvents;

	void setListens(const std::vector<Listen> &lists);
	void setDefaultErrorPage(const std::string &page);
	void setMaxRequestBodySize(unsigned int size);
	void setIsAllowGet(bool allow);
	void setIsAllowPost(bool allow);
	void setIsAllowHead(bool allow);
	void setIsAllowDelete(bool allow);
	void setRedirect(const std::string &url);
	void setLocations(const std::vector<Location> &url);
	void setIsShowDirectoryListPage(bool show);
	void setWhenRequestedDirectory(const std::string &dir);
	void setSaveFileDirectory(const std::string &dir);

public:
	Config(const std::string &configFile);
	~Config();
	Config(const Config &other);
	Config &operator=(const Config &other);

	void setup(const std::string& configFile);

	const std::vector<Listen> &getListens() const;
	const std::string &getDefaultErrorPage() const;
	unsigned int getMaxRequestBodySize() const;
	bool getIsAllowGet() const;
	bool getIsAllowPost() const;
	bool getIsAllowHead() const;
	bool getIsAllowDelete() const;
	const std::string &getRedirect() const;
	const std::vector<Location> &getLocations() const;
	bool getIsShowDirectoryListPage() const;
	const std::string &getWhenRequestedDirectory() const;
	const std::string &getSaveFileDirectory() const;
	unsigned int getTimeoutSec() const;
	unsigned int getMaxEvents() const;

	friend std::ostream &operator<<(std::ostream &os, const Config &config);
};
