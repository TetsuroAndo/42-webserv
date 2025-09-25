#pragma once

#include <string>
#include <map>

class MimeType {
public:
	static std::string getMimeType(const std::string& extension);

private:
	static std::map<std::string, std::string> _mimeMap;
	static void _setMimeTypes();
	static bool _isInitialized;

	MimeType();
	MimeType(const MimeType&);
	MimeType& operator=(const MimeType&);
	~MimeType();

};
