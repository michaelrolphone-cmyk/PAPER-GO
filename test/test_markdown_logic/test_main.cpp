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

void test_render_preview_formats_structural_lines() {
  String md = "# Title\n\n- Item one\n> Quote\n```\ncode line\n```\n";
  String out = markdownRenderPreview(md, 500, 12, 30);
  TEST_ASSERT_TRUE(out.indexOf("[H] Title") >= 0);
  TEST_ASSERT_TRUE(out.indexOf("• Item one") >= 0);
  TEST_ASSERT_TRUE(out.indexOf("> Quote") >= 0);
  TEST_ASSERT_TRUE(out.indexOf("| code line") >= 0);
}

void test_render_preview_wraps_long_lines() {
  String md = "This is a very long markdown line that should wrap across multiple preview rows.";
  String out = markdownRenderPreview(md, 500, 10, 20);
  int firstNl = out.indexOf('\n');
  TEST_ASSERT_TRUE(firstNl > 0);
  TEST_ASSERT_TRUE(out.substring(0, firstNl).length() <= 20);
}

void test_render_preview_supports_ordered_links_and_images() {
  String md = "1. First [link](https://example.com)\n2. ![map](x.png)\n|h|v|\n";
  String out = markdownRenderPreview(md, 500, 12, 40);
  TEST_ASSERT_TRUE(out.indexOf("1. First link") >= 0);
  TEST_ASSERT_TRUE(out.indexOf("2. [Image: map]") >= 0);
  TEST_ASSERT_TRUE(out.indexOf("[T] |h|v|") >= 0);
}

void test_render_preview_supports_task_lists_and_rule_variants() {
  String md = "- [ ] Pending task\n- [x] Done task\n___\n";
  String out = markdownRenderPreview(md, 500, 12, 40);
  TEST_ASSERT_TRUE(out.indexOf("☐ Pending task") >= 0);
  TEST_ASSERT_TRUE(out.indexOf("☑ Done task") >= 0);
  TEST_ASSERT_TRUE(out.indexOf("----------------") >= 0);
}

void test_markdown_window_scroll_helpers() {
  String rendered = "l1\nl2\nl3\nl4\nl5\n";
  TEST_ASSERT_EQUAL(2, markdownClampStartLine(rendered, 99, 3));
  TEST_ASSERT_EQUAL(0, markdownClampStartLine(rendered, -4, 3));
  TEST_ASSERT_EQUAL(5, (int)markdownLineCount(rendered));
  TEST_ASSERT_EQUAL(1, (int)markdownLineCount("single-line"));
  String win = markdownWindow(rendered, 1, 2);
  TEST_ASSERT_EQUAL_STRING("l2\nl3\n", win.c_str());
}

void test_markdown_progress_state_round_trip() {
  String path = "/documents/markdown/readme.md";
  String state = markdownBuildProgressState(path, 12);
  TEST_ASSERT_TRUE(state.indexOf("\"path\":\"/documents/markdown/readme.md\"") >= 0);
  TEST_ASSERT_EQUAL(12, markdownReadProgressStartLine(state, path, 0));
}

void test_markdown_progress_state_round_trip_with_json_escaped_chars() {
  String path = "/documents/markdown/quotes-\"and\"-slashes-\\\\.md";
  String state = markdownBuildProgressState(path, 4);
  TEST_ASSERT_EQUAL(4, markdownReadProgressStartLine(state, path, 0));
}

void test_markdown_progress_state_mismatch_returns_default() {
  String state = "{\"path\":\"/documents/markdown/other.md\",\"startLine\":9}";
  TEST_ASSERT_EQUAL(3, markdownReadProgressStartLine(state, "/documents/markdown/readme.md", 3));
}

void test_markdown_progress_state_invalid_json_returns_default() {
  String state = "{\"path\":\"/documents/markdown/readme.md\",\"startLine\":";
  TEST_ASSERT_EQUAL(7, markdownReadProgressStartLine(state, "/documents/markdown/readme.md", 7));
}

void test_markdown_progress_state_clamps_negative_line_to_zero() {
  String state = "{\"path\":\"/documents/markdown/readme.md\",\"startLine\":-5}";
  TEST_ASSERT_EQUAL(0, markdownReadProgressStartLine(state, "/documents/markdown/readme.md", 7));
}

int main(int argc, char** argv) {
  UNITY_BEGIN();
  RUN_TEST(test_markdown_title_from_heading);
  RUN_TEST(test_markdown_title_plain_first_line);
  RUN_TEST(test_preview_limits_lines);
  RUN_TEST(test_render_preview_formats_structural_lines);
  RUN_TEST(test_render_preview_wraps_long_lines);
  RUN_TEST(test_render_preview_supports_ordered_links_and_images);
  RUN_TEST(test_render_preview_supports_task_lists_and_rule_variants);
  RUN_TEST(test_markdown_window_scroll_helpers);
  RUN_TEST(test_markdown_progress_state_round_trip);
  RUN_TEST(test_markdown_progress_state_round_trip_with_json_escaped_chars);
  RUN_TEST(test_markdown_progress_state_mismatch_returns_default);
  RUN_TEST(test_markdown_progress_state_invalid_json_returns_default);
  RUN_TEST(test_markdown_progress_state_clamps_negative_line_to_zero);
  return UNITY_END();
}
