#include "../../src/Config/Config.hpp"
#include "../../src/Lib/Logger/AccessLog/AccessLogger.hpp"
#include "../../src/Http/Core/HttpRequest.hpp"
#include "../../src/Http/Core/HttpResponse.hpp"
#include <iostream>
#include <cstdlib> // for exit()

void setupLogger(const std::string &configFile) {
    try {
        Config config(configFile);
        const std::vector<AccessLog> &accessLogs = config.getAccessLogs();
        if (!accessLogs.empty()) {
            const AccessLog &logConf = accessLogs[0];
            AccessLogger::getInstance().setSinkFile(logConf.logDir, logConf.filename, logConf.format, logConf.maxFileSize, logConf.maxBackupFiles);
        }
    } catch (const std::exception &e) {
        std::cerr << "Config Error: " << e.what() << std::endl;
        exit(1);
    }
}

int main(int argc, char **argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <config_file>" << std::endl;
        return 1;
    }

    setupLogger(argv[1]);

    HttpRequest req;
    req.setMethod("GET");
    req.setPath("/rotation-test-path");
    req.setVersion("HTTP/1.1");
    req.addHeader("User-Agent", "LogRotationSimpleTest/1.0");
    req.addHeader("Host", "localhost");

    HttpResponse res("log-test-server");
    res.setStatusCode(200);
    res.setHeader("Content-Length", "123");
    res.setHeader("Content-Type", "text/plain");

    std::cout << "Writing 12 log lines to trigger one rotation..." << std::endl;

    for (int i = 0; i < 12; ++i) {
        AccessLogger::getInstance().log(&req, &res, "127.0.0.1", 12345, "session-id-12345");
    }

    std::cout << "Finished writing logs." << std::endl;

    return 0;
}