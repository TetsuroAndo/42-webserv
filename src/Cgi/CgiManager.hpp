#pragma once

#include <string>
#include <map>
#include <vector>
#include "CgiWorker.hpp"
#include "../Socket/SocketsManager.hpp"

class CgiManager {
public:
	static CgiManager& getInstance();
	~CgiManager();

	// Client -> CgiHandlerから呼び出される
	CgiWorker* createWorker(const HttpRequest& req, const Location& locConf, const std::string& scriptPath, SocketsManager& socketManager);

	// メインループからイベント発生時に呼ばれる
	void handleEvent(int fd, SocketsManager& socketManager);

	// メインループから定期的に呼ばれる
	void cleanupWorkers(SocketsManager& socketManager);

private:
	CgiManager();
	CgiManager(const CgiManager&);
	CgiManager& operator=(const CgiManager&);

	std::map<int, CgiWorker*> _fdToWorker;
	std::vector<CgiWorker*> _workers;
};
