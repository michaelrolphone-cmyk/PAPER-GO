#pragma once
#include <Arduino.h>

struct AppManagementRequest {
  String id;
  String sourceUrl;
  String version;
};

bool parseAppManagementRequest(const String& body, AppManagementRequest& out);
String buildAppManagementResultJson(const char* action, const String& appId, bool ok, const String& detail);
bool isValidAppId(const String& appId);
