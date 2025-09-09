#ifndef HTTPPARSER_HPP
#define HTTPPARSER_HPP

#include <map>
#include <string>

class HttpRequest {
	private:
		bool _complete;
		const static size_t maxBodySize = 10 * 1024 * 1024;
		const static size_t maxHeaderSize = 8192;

		std::string _method;
		std::string _path;
		std::string _version;
		std::map<std::string, std::string> _headers;
		std::map<std::string, std::string> _query;
		std::string _body;

		bool parseRequestLine(std::string& requestLine);
		bool parseHeaders(std::istringstream& headerStream);
		bool parseBody(std::string& buffer, size_t bodyStart);
	public:
		HttpRequest();
		~HttpRequest();

		bool parse(std::string &buffer); 
		bool isComplete() const;

		const std::string &getMethod() const;
		const std::string &getPath() const;
		const std::string &getVersion() const;
		const std::string &getBody() const;
		const std::string &getHeader(const std::string &header) const;

		void printData();
};

#endif
