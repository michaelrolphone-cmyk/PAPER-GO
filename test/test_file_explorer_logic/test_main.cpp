#include <unity.h>
#include "FileExplorerLogic.h"

void test_sort_dirs_first_then_name() {
  std::vector<FileEntryView> v = {
    {"b.txt", false, 10},
    {"alpha", true, 0},
    {"a.txt", false, 5},
    {"zeta", true, 0}
  };
  sortFileEntries(v);
  TEST_ASSERT_TRUE(v[0].isDir);
  TEST_ASSERT_TRUE(v[1].isDir);
  TEST_ASSERT_EQUAL_STRING("alpha", v[0].name.c_str());
  TEST_ASSERT_EQUAL_STRING("zeta", v[1].name.c_str());
  TEST_ASSERT_EQUAL_STRING("a.txt", v[2].name.c_str());
}

void test_format_entry() {
  FileEntryView d{"docs", true, 0};
  TEST_ASSERT_EQUAL_STRING("[D] docs  0 bytes", formatFileEntry(d).c_str());
}

void test_parent_path() {
  TEST_ASSERT_EQUAL_STRING("/", parentPath("/").c_str());
  TEST_ASSERT_EQUAL_STRING("/", parentPath("/docs").c_str());
  TEST_ASSERT_EQUAL_STRING("/docs", parentPath("/docs/work").c_str());
}

void test_join_path() {
  TEST_ASSERT_EQUAL_STRING("/alpha", joinPath("/", "alpha").c_str());
  TEST_ASSERT_EQUAL_STRING("/docs/work", joinPath("/docs", "work").c_str());
}

void test_file_base_name() {
  TEST_ASSERT_EQUAL_STRING("a.txt", fileBaseName("/documents/a.txt").c_str());
  TEST_ASSERT_EQUAL_STRING("plain", fileBaseName("plain").c_str());
}

int main(int argc, char** argv) {
  UNITY_BEGIN();
  RUN_TEST(test_sort_dirs_first_then_name);
  RUN_TEST(test_format_entry);
  RUN_TEST(test_parent_path);
  RUN_TEST(test_join_path);
  RUN_TEST(test_file_base_name);
  return UNITY_END();
}
