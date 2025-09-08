#pragma once
#include <ostream>
#include <string>
#include <vector>

struct listen {
	std::string interface;
	int port;
};

struct location {
	std::string path;
	std::string root;
};

class Config {
private:
	std::vector<listen> _listens;
	std::string _defaultErrorPage;
	unsigned int _maxRequestBodySize;
	bool isAllowGet;
	bool isAllowPost;
	bool isAllowHead;
	bool isAllowDelete;
	std::string redirect;
	std::vector<location> locations;
	bool isShowDirectoryListPage;
	std::string whenRequestedDirectory;
	std::string saveFileDirectory;
	unsigned int timeoutSec;
	unsigned int maxEvents;

	void setListens(const std::vector<listen> &lists);
	void setDefaultErrorPage(const std::string &page);
	void setMaxRequestBodySize(unsigned int size);
	void setIsAllowGet(bool allow);
	void setIsAllowPost(bool allow);
	void setIsAllowHead(bool allow);
	void setIsAllowDelete(bool allow);
	void setRedirect(const std::string &url);
	void setLocations(const std::vector<location> &url);
	void setIsShowDirectoryListPage(bool show);
	void setWhenRequestedDirectory(const std::string &dir);
	void setSaveFileDirectory(const std::string &dir);

public:
	Config();
	~Config();
	Config(const Config &other);
	Config &operator=(const Config &other);

	void setup(const std::string& configFile = "");

	const std::vector<listen> &getListens() const;
	const std::string &getDefaultErrorPage() const;
	unsigned int getMaxRequestBodySize() const;
	bool getIsAllowGet() const;
	bool getIsAllowPost() const;
	bool getIsAllowHead() const;
	bool getIsAllowDelete() const;
	const std::string &getRedirect() const;
	const std::vector<location> &getLocations() const;
	bool getIsShowDirectoryListPage() const;
	const std::string &getWhenRequestedDirectory() const;
	const std::string &getSaveFileDirectory() const;
	unsigned int getTimeoutSec() const;
	unsigned int getMaxEvents() const;

	friend std::ostream &operator<<(std::ostream &os, const Config &config);
};
