#include "WebUtils.h"

String mimeTypeForPath(const String& path) {
  if (path.endsWith(".html") || path.endsWith(".htm")) return "text/html";
  if (path.endsWith(".css")) return "text/css";
  if (path.endsWith(".js")) return "application/javascript";
  if (path.endsWith(".json")) return "application/json";
  if (path.endsWith(".txt") || path.endsWith(".md")) return "text/plain";
  if (path.endsWith(".png")) return "image/png";
  if (path.endsWith(".jpg") || path.endsWith(".jpeg")) return "image/jpeg";
  if (path.endsWith(".svg")) return "image/svg+xml";
  return "application/octet-stream";
}

bool isSafeWebUri(const String& uri) {
  if (!uri.startsWith("/")) return false;
  if (uri.indexOf("..") >= 0) return false;
  if (uri.indexOf('\\') >= 0) return false;
  return true;
}

String mapUriToWebrootPath(const String& uri) {
  if (uri == "/") return "/webroot/index.html";
  return "/webroot" + uri;
}
