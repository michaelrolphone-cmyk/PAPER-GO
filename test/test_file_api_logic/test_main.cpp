#include <unity.h>
#include "FileApiLogic.h"

void test_path_normalize_and_allow() {
  TEST_ASSERT_EQUAL_STRING("/documents/a.txt", normalizeFilePath("documents//a.txt").c_str());
  TEST_ASSERT_FALSE(isAllowedFilePath("/etc/passwd"));
  TEST_ASSERT_TRUE(isAllowedFilePath("/documents/a.txt"));
  TEST_ASSERT_FALSE(isAllowedFilePath("/documents/../secret"));
  TEST_ASSERT_EQUAL_STRING("", normalizeFilePath("/documents\\..\\secret.txt").c_str());
  TEST_ASSERT_FALSE(isAllowedFilePath("/documents\\secret.txt"));
}

void test_file_api_json_shapes() {
  std::vector<FileListEntry> entries;
  FileListEntry e; e.name="a.txt"; e.path="/documents/a.txt"; entries.push_back(e);
  String j = fileListJson("/documents", 0, 100, 1, entries);
  TEST_ASSERT_TRUE(j.indexOf("\"entries\"") >= 0);
  TEST_ASSERT_TRUE(fileApiErrorJson("forbidden_path","x").indexOf("\"ok\":false") >= 0);
}

int main(int argc, char** argv){UNITY_BEGIN(); RUN_TEST(test_path_normalize_and_allow); RUN_TEST(test_file_api_json_shapes); return UNITY_END();}
