#pragma once

#include "Config.hpp"
#include <string>

class Builder;
class Node;

class ConfigBuilder {
private:
	std::vector<Listen> _listens;
	std::map<std::string, Redirect> _redirects;
	std::map<std::string, Location> _locations;
	std::vector<AccessLog> _accessLogs;
	std::vector<ErrorLog> _errorLogs;
	unsigned int _maxRequestBodySize;
	unsigned int _timeoutSec;
	unsigned int _maxEvents;

	void initDefaults();
	void setup(const std::string &configFile);

public:
	ConfigBuilder();
	ConfigBuilder(const std::string &configFile);
	~ConfigBuilder();

	Config build() const;

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
	void setAccessLogs(const std::vector<AccessLog> &accessLogs);
	void setErrorLogs(const std::vector<ErrorLog> &errorLogs);
	void setRedirects(const std::map<std::string, Redirect> &redirects);
	void setRedirect(const Redirect &redirect,
					 const std::string &redirectKey = "/");
	void setLocations(const std::map<std::string, Location> &locations);
	void setLocation(const Location &location,
					 const std::string &locationKey = "/");

	void setMaxRequestBodySize(unsigned int size);
	void setTimeoutSec(unsigned int sec);
	void setMaxEvents(unsigned int maxEvents);

	const Location &getLocation(const std::string &key) const;
};
