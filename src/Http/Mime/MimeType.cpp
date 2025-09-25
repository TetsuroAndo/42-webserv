#include "../../Lib/StringOps/StringOps.hpp"
#include "MimeType.hpp"
#include <algorithm>
#include <map>

std::map<std::string, std::string> MimeType::_mimeMap;
bool MimeType::_isInitialized = false;

void MimeType::_setMimeTypes() {
	_mimeMap[".html"] = "text/html";
	_mimeMap[".htm"] = "text/html";
	_mimeMap[".css"] = "text/css";
	_mimeMap[".js"] = "application/javascript";
	_mimeMap[".json"] = "application/json";
	_mimeMap[".xml"] = "application/xml";
	_mimeMap[".pdf"] = "application/pdf";
	_mimeMap[".zip"] = "application/zip";
	_mimeMap[".txt"] = "text/plain";

	// Images
	_mimeMap[".jpeg"] = "image/jpeg";
	_mimeMap[".jpg"] = "image/jpeg";
	_mimeMap[".png"] = "image/png";
	_mimeMap[".gif"] = "image/gif";
	_mimeMap[".bmp"] = "image/bmp";
	_mimeMap[".ico"] = "image/x-icon";
	_mimeMap[".svg"] = "image/svg+xml";

	// Audio/Video
	_mimeMap[".mp3"] = "audio/mpeg";
	_mimeMap[".mp4"] = "video/mp4";
	_mimeMap[".webm"] = "video/webm";
	_mimeMap[".wav"] = "audio/wav";

	// Fonts
	_mimeMap[".ttf"] = "font/ttf";
	_mimeMap[".otf"] = "font/otf";
	_mimeMap[".woff"] = "font/woff";
	_mimeMap[".woff2"] = "font/woff2";
	_mimeMap[".eot"] = "application/vnd.ms-fontobject";

	_isInitialized = true;
}

std::string MimeType::getMimeType(const std::string& extension) {
	if (!_isInitialized) {
		_setMimeTypes();
	}

	size_t dot_pos = extension.find_last_of('.');
	if (dot_pos == std::string::npos) {
		return "application/octet-stream";
	}
	std::string ext = extension.substr(dot_pos);
	StringOps::toLower(ext);

	std::map<std::string, std::string>::const_iterator it = _mimeMap.find(ext);
	if (it != _mimeMap.end()) {
		return it->second;
	}
	return "application/octet-stream";
}
