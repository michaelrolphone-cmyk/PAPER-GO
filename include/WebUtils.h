#pragma once
#include <Arduino.h>

String mimeTypeForPath(const String& path);
bool isSafeWebUri(const String& uri);
String mapUriToWebrootPath(const String& uri);
