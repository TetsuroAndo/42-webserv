#include "../CgiEnvironmentBuilder.hpp"
#include "../../Config/Config.hpp"
#include "../../Http/Core/HttpRequest.hpp"
#include "DummyHttpReq.hpp"
#include <iostream>
#include <sstream>
#include <cassert>
#include <map>
#include <vector>

void assert_env_var(const std::vector<std::string>& envp, const std::string& key, const std::string& expected_value) {
	const std::string full_expected = key + "=" + expected_value;
	const std::string prefix = key + "=";
	//bool found = false;

	for (size_t i = 0; i < envp.size(); ++i) {
		// envp[i]がprefixで始まるかをチェック (C++98のrfindの引数に注意)
		if (envp[i].size() >= prefix.size() && envp[i].rfind(prefix, 0) == 0) {
			//found = true;
			if (envp[i] == full_expected) {
				std::cout << "\033[32m[PASS]\033[0m " << full_expected << std::endl;
				return;
			} else {
				// キーは一致したが、値が不一致の場合
				std::cerr << "\033[31m[ERROR]\033[0m Expected: " << full_expected
						<< ", Got: " << envp[i] << std::endl;
				assert(false); // 値が一致しない場合はアボート
			}
		}
	}
    // 環境変数が見つからなかった場合
	std::cerr << "\033[31m[FAIL]\033[0m " << key << " not found in envp" << std::endl;
	assert(false); // 見つからなかった場合はアボート
}

void testCgiEnvironmentBuilder() {
	std::cout << "\n--- Testing CgiEnvironmentBuilder ---\n" << std::endl;
	Location locConf; 
	std::string scriptPath = "/var/www/cgi-bin/test.py";

	// テストケース1: GETリクエストとクエリ
	{
		std::map<std::string, std::string> headers;
		headers["User-Agent"] = "WebservTest";
		headers["Accept"] = "text/html";
		// HTTPヘッダーキーは小文字で格納されていることに注意
		std::map<std::string, std::string> queries;
		queries["id"] = "123";
		queries["name"] = "test";

		// DummyHttpReq::getMethod()は"GET"を返す
		DummyHttpReq req("GET", "/cgi/test.py", headers, "", queries);
		
		std::vector<std::string> envp = CgiEnvironmentBuilder::build(req, locConf, scriptPath);

		// REQUEST_METHODのテスト (失敗していた箇所)
		assert_env_var(envp, "REQUEST_METHOD", "GET"); 
		assert_env_var(envp, "SCRIPT_FILENAME", scriptPath);
		assert_env_var(envp, "SCRIPT_NAME", "/cgi/test.py");
		assert_env_var(envp, "QUERY_STRING", "id=123&name=test");
		assert_env_var(envp, "CONTENT_LENGTH", "0");
		// ヘッダーはCgiEnvironmentBuilderで大文字に変換される
		assert_env_var(envp, "HTTP_USER_AGENT", "WebservTest");
		assert_env_var(envp, "HTTP_ACCEPT", "text/html");
	}

	// テストケース2: POSTリクエストとボディ
	{
		std::map<std::string, std::string> headers;
		headers["Content-Type"] = "application/x-www-form-urlencoded";
		std::string body = "key1=value1&key2=value2";
		
		DummyHttpReq req("POST", "/cgi/upload.php", headers, body, std::map<std::string, std::string>());
		
		std::vector<std::string> envp = CgiEnvironmentBuilder::build(req, locConf, scriptPath);
		
		assert_env_var(envp, "REQUEST_METHOD", "POST");
		assert_env_var(envp, "CONTENT_TYPE", "application/x-www-form-urlencoded");
		std::stringstream ss;
		ss << body.length();
		assert_env_var(envp, "CONTENT_LENGTH", ss.str()); 
	}
}

int main() {
	testCgiEnvironmentBuilder();
	return 0;
}
