#include "FileApiLogic.h"

static const char* kRoots[] = {"/apps","/cache","/documents","/games","/gps","/meshtastic","/radio","/config","/logs","/webroot"};

String normalizeFilePath(const String& path) {
  if (!path.length()) return "";
  String p = path;
  if (!p.startsWith("/")) p = "/" + p;
  while (p.indexOf("//") >= 0) p.replace("//", "/");
  if (p.indexOf("..") >= 0) return "";
  return p;
}

bool isAllowedFilePath(const String& path) {
  String p = normalizeFilePath(path);
  if (!p.length()) return false;
  for (auto r : kRoots) {
    if (p == r || p.startsWith(String(r) + "/")) return true;
  }
  return false;
}

String fileApiErrorJson(const String& code, const String& message) {
  return String("{\"ok\":false,\"error\":{\"code\":\"") + code + "\",\"message\":\"" + message + "\"}}";
}
String fileApiOkJson(const String& dataJson) {
  return String("{\"ok\":true,\"data\":") + dataJson + "}";
}
String fileListJson(const String& path, int page, int pageSize, int total, const std::vector<FileListEntry>& entries) {
  String j = "{";
  j += "\"path\":\"" + path + "\",\"page\":" + String(page) + ",\"pageSize\":" + String(pageSize) + ",\"total\":" + String(total) + ",\"entries\":[";
  bool first = true;
  for (auto &e : entries) {
    if (!first) j += ",";
    first = false;
    j += "{\"name\":\"" + e.name + "\",\"path\":\"" + e.path + "\",\"dir\":" + String(e.dir?"true":"false") + ",\"size\":" + String(e.size) + "}";
  }
  j += "]}";
  return j;
}
