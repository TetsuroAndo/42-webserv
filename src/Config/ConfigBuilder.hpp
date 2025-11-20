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
	std::map< int, std::string > _errorPages;
	size_t _maxRequestBodySize;
	bool _hasBiggestRequestBodySize;
	size_t _biggestRequestBodySize;
	size_t _timeoutSec;
	size_t _maxEvents;
	std::string _defaultLocationKey;
	size_t _requestHeaderTimeoutSec;
	size_t _requestBodyTimeoutSec;
	size_t _sessionTimeoutSec;

	void initDefaults();
	void setup(const std::string &configFile);

public:
	ConfigBuilder();
	ConfigBuilder(const std::string &configFile);
	~ConfigBuilder();

	Config build() const;

	// グローバル設定
	void setMaxRequestBodySize(size_t size);
	void setBiggestRequestBodySize(bool hasValue, size_t size);
	void setTimeoutSec(size_t sec);
	void setMaxEvents(size_t maxEvents);
	void setListens(const std::vector< Listen > &lists);
	void setAccessLogs(const std::vector< AccessLog > &accessLogs);
	void setErrorLogs(const std::vector< ErrorLog > &errorLogs);
	void setErrorPage(int code, const std::string &uri);
	void setLocations(const std::map< std::string, Location > &locations);
	void setLocation(const Location &location);
	void setSessionTimeoutSec(size_t sec);

	// サーバーブロック直下の設定（デフォルトロケーション）
	void setServerDefaultRoot(const std::string &root);
	void setServerDefaultAutoindex(bool autoindex);
	void setServerDefaultindex(
		const std::string &index); // TODO: 名前ミス直す　Iを大文字にする
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
	void setRequestHeaderTimeoutSec(size_t sec);
	void setRequestBodyTimeoutSec(size_t sec);
};
