#include <unity.h>
#include "MarkdownLogic.h"

void test_markdown_title_from_heading() {
  String md = "# Field Notes\n\nhello";
  TEST_ASSERT_EQUAL_STRING("Field Notes", markdownTitle(md).c_str());
}

void test_markdown_title_plain_first_line() {
  String md = "First line\n# heading";
  TEST_ASSERT_EQUAL_STRING("First line", markdownTitle(md).c_str());
}

void test_preview_limits_lines() {
  String md = "a\nb\nc\nd\n";
  String out = markdownPreview(md, 100, 2);
  TEST_ASSERT_TRUE(out.indexOf("a\nb\n") == 0);
  TEST_ASSERT_TRUE(out.indexOf("c") < 0);
}

int main(int argc, char** argv) {
  UNITY_BEGIN();
  RUN_TEST(test_markdown_title_from_heading);
  RUN_TEST(test_markdown_title_plain_first_line);
  RUN_TEST(test_preview_limits_lines);
  return UNITY_END();
}
