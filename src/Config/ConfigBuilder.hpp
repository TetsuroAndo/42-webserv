#pragma once

#include "Config.hpp"
#include <string>

class Builder;
class Node;

class ConfigBuilder {
private:
	std::vector< Listen > _listens;
	std::map< std::string, Location > _locations;
	std::vector< AccessLog > _accessLogs;
	std::vector< ErrorLog > _errorLogs;
	std::map<int, std::string> _errorPages;
	unsigned int _maxRequestBodySize;
	unsigned int _timeoutSec;
	unsigned int _maxEvents;
	std::string _defaultLocationKey;
	unsigned int _requestHeaderTimeoutSec;
	unsigned int _requestBodyTimeoutSec;

	void initDefaults();
	void setup(const std::string &configFile);

public:
	ConfigBuilder();
	ConfigBuilder(const std::string &configFile);
	~ConfigBuilder();

	Config build() const;

	// グローバル設定
	void setMaxRequestBodySize(unsigned int size);
	void setTimeoutSec(unsigned int sec);
	void setMaxEvents(unsigned int maxEvents);
	void setListens(const std::vector< Listen > &lists);
	void setAccessLogs(const std::vector< AccessLog > &accessLogs);
	void setErrorLogs(const std::vector< ErrorLog > &errorLogs);
	void setErrorPage(int code, const std::string &uri);
	void setLocations(const std::map< std::string, Location > &locations);
	void setLocation(const Location &location);

	// サーバーブロック直下の設定（デフォルトロケーション）
	void setServerDefaultRoot(const std::string &root);
	void setServerDefaultAutoindex(bool autoindex);
	void setServerDefaultIndexFile(const std::string &indexFile);
	void setServerDefaultUploadStore(const std::string &uploadStore);
	void setServerDefaultCgiConf(const std::string &extension,
								 const std::string &interpreterPath);
	void setServerDefaultIsAllowGet(bool allow);
	void setServerDefaultIsAllowHead(bool allow);
	void setServerDefaultIsAllowPost(bool allow);
	void setServerDefaultIsAllowDelete(bool allow);
	void setServerDefaultAllowedMethods(const std::string &methods);
	void setServerDefaultAllowedMethods(const std::set< std::string > &methods);
	void setServerDefaultSession(bool enable);
	void setRequestHeaderTimeoutSec(unsigned int sec);
	void setRequestBodyTimeoutSec(unsigned int sec);
};
