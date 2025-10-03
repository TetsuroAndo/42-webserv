#include "../../../Http/Core/HttpRequest.hpp"
#include "../../../Http/Core/HttpResponse.hpp"
#include "../AccessLog/AccessLogger.hpp"
#include "../ErrorLog/Logger.hpp"
#include <iostream>
#include <stdexcept>
#include <map>
#include <string>
#include <vector>
#include <unistd.h>

// ========================================================================
// AccessLoggerのテストに必要なHttpRequest/HttpResponseのモック(ダミークラス)
// ========================================================================
class MockHttpRequest : public HttpRequest {
private:
	std::string _method;
	std::string _path;
	std::string _version;
	std::map<std::string, std::string> _queries;
	std::map<std::string, std::string> _headers;

public:
	virtual ~MockHttpRequest() {}

	MockHttpRequest(const std::string& method, const std::string& path,
					const std::map<std::string, std::string>& queries,
					const std::map<std::string, std::string>& headers,
					const std::string& version = "HTTP/1.1")
		: _method(method), _path(path), _version(version), _queries(queries),
		  _headers(headers) {}

	const std::string& getMethod() const { return _method; }
	const std::string& getPath() const { return _path; }
	const std::string& getVersion() const { return _version; }
	const std::map<std::string, std::string>& getQueries() const { return _queries; }
	const std::string& getHeader(const std::string& key) const {
		std::map<std::string, std::string>::const_iterator it = _headers.find(key);
		if (it != _headers.end()) {
			return it->second;
		}
		static const std::string empty_string = "";
		return empty_string;
	}
};

// 3. HttpResponseをpublic継承する
class MockHttpResponse : public HttpResponse {
public:
	virtual ~MockHttpResponse() {}

	MockHttpResponse(int statusCode, const std::string& body)
		: HttpResponse(SERVER_NAME)
	{
		setStatusCode(statusCode);
		setBody(body);
	}
};

// ========================================================================
// テスト用ヘルパー関数
// ========================================================================
void printHeader(const std::string& title) {
	std::cout << "\n--- " << title << " ------------------------------------\n" << std::endl;
}

// ========================================================================
// ErrorLogger テスト
// ========================================================================
void testErrorLogger() {
	std::cout << "Setting up ErrorLogger sinks..." << std::endl;
	Logger& logger = Logger::getInstance();

	// テスト用のログディレクトリを設定
	logger.setLogDir("./test_log");

	// 1. コンソール出力 (JSON形式, DEBUGレベル以上)
	logger.setSinkConsole(JSON, DEBUG);

	// 2. ファイル出力 (ELF形式, INFOレベル以上)
	logger.setSinkFile("error_elf.log", ELF, INFO);

	// 3. ファイル出力 (JSON形式, WARNINGレベル以上)
	logger.setSinkFile("error_json_warn.log", JSON, WARNING);

	// 4. ファイル出力 (ELF形式, ERRORレベルのみ)
	logger.setSinkFile("error_elf_exact.log", ELF, ERROR, EXACT);

	// 5. ローテーションテスト用ファイル (小さなファイルサイズに設定)
	logger.setSinkFile("rotation.log", ELF, DEBUG, GREATER_OR_EQUAL, 200, 3);

	std::cout << "Sending log messages to ErrorLogger..." << std::endl;

	LOG(DEBUG) << "This is a debug message. Should appear on console and rotation.log.";

	LOG(INFO) << "Server started successfully."
			  << attr("host", "localhost")
			  << attr("port", 8080);

	LOG(WARNING) << "Deprecated configuration option detected."
				 << attr("option", "old_feature")
				 << attr("advice", "Use new_feature instead");

	LOG(ERROR) << "Failed to connect to database."
			   << attr("db_host", "db.example.com")
			   << attr("reason", "Connection timeout");

	LOG(FATAL) << "Critical error: Out of memory."
			   << attr("component", "MemoryAllocator");

	std::cout << "\nTesting log rotation for 'rotation.log'..." << std::endl;
	for (int i = 0; i < 10; ++i) {
		LOG(DEBUG) << "Log rotation test message #" << i + 1;
	}
	std::cout << "Rotation test messages sent. Check for rotation.log.1, .2, etc." << std::endl;
}


// ========================================================================
// AccessLogger テスト
// ========================================================================
void testAccessLogger() {
	std::cout << "Setting up AccessLogger sinks..." << std::endl;
	AccessLogger& accessLogger = AccessLogger::getInstance();

	// テスト用のログディレクトリを設定
	accessLogger.setLogDir("./test_log");

	// 1. コンソール出力 (ELF形式)
	accessLogger.setSinkConsole(ELF);
	// 2. ファイル出力 (JSON形式)
	accessLogger.setSinkFile("access_json.log", JSON);
	accessLogger.setSinkFile("access_elf.log", ELF);

	std::cout << "Sending log messages to AccessLogger..." << std::endl;

	// --- Test Case 1: シンプルなGETリクエスト ---
	{
		std::map<std::string, std::string> queries;
		std::map<std::string, std::string> headers;
		headers["User-Agent"] = "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/108.0.0.0 Safari/537.36";
		headers["Referer"] = "http://example.com/link_page.html";

		MockHttpRequest req("GET", "/index.html", queries, headers);
		MockHttpResponse res(200, "<html><body>Hello</body></html>");

		// 修正点：集成体初期化を使ってconstメンバを初期化する
		AccessLogContext ctx = {
			std::time(NULL),      // time_t timestamp
			&req,                 // const HttpRequest* request
			&res,                 // const HttpResponse* response
			"192.168.1.10",       // std::string remote_addr
			54321,                // int client_port
			"a1b2c3d4e5f6"        // std::string session_id
		};

		accessLogger.log(ctx);
	}

	// タイムスタンプが確実に変わるように少し待つ
	sleep(1);

	// --- Test Case 2: クエリ付きPOSTリクエスト (404 Not Found) ---
	{
		std::map<std::string, std::string> queries;
		queries["id"] = "123";
		queries["action"] = "delete";
		std::map<std::string, std::string> headers;
		headers["User-Agent"] = "curl/7.68.0";
		// Refererなし

		MockHttpRequest req("POST", "/api/resource", queries, headers);
		MockHttpResponse res(404, "Resource not found.");

		// 修正点：こちらも集成体初期化を使用
		AccessLogContext ctx = {
			std::time(NULL),     // time_t timestamp
			&req,                // const HttpRequest* request
			&res,                // const HttpResponse* response
			"10.0.0.5",          // std::string remote_addr
			12345,               // int client_port
			""                   // std::string session_id (セッションなし)
		};

		accessLogger.log(ctx);
	}
}

// ========================================================================
// Main
// ========================================================================
int main() {
	try {
		testErrorLogger();
		testAccessLogger();

		printHeader("All tests completed");
		std::cout << "Please check the console output and the files in './test_log/' directory:" << std::endl;
		std::cout << "  - error_elf.log (INFO+)" << std::endl;
		std::cout << "  - error_json_warn.log (WARNING+)" << std::endl;
		std::cout << "  - error_elf_exact.log (ERROR only)" << std::endl;
		std::cout << "  - rotation.log, rotation.log.1, ... (Log rotation test)" << std::endl;
		std::cout << "  - access_elf.log (Access log in ELF format)" << std::endl;
		std::cout << "  - access_json.log (Access log in JSON)" << std::endl;
		std::cout << "\nNOTE: Access logs in ELF format are also printed to the console." << std::endl;
		std::cout << "========================================================" << std::endl;

	} catch (const std::exception& e) {
		std::cerr << "\n[FATAL] A critical error occurred during testing: " << e.what() << std::endl;
		return 1;
	}
	return 0;
}
