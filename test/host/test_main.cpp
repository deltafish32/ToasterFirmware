// Host unit tests for the firmware's platform-independent code (YAML parser, string utilities).
//
// Usage:
//   host_test              run all tests
//   host_test NAME         run only the tests whose name contains NAME
//   host_test --dump FILE  print the parsed structure of a YAML file (for comparing parser versions)
//
// Build and run with run.ps1 (MSVC) or run.sh (g++/clang).

#include <stdio.h>
#include <string.h>
#include <string>
#include <vector>
#include <functional>

#include "yaml_array.h"
#include "yaml_node.h"
#include "strutil.h"

using namespace toaster;


// ---------------------------------------------------------------------------
// minimal test harness

static int g_checks = 0;
static int g_failures = 0;
static const char* g_current_test = "";

struct TestCase {
  const char* name;
  std::function<void()> func;
};

static std::vector<TestCase>& tests() {
  static std::vector<TestCase> list;
  return list;
}

struct TestRegistrar {
  TestRegistrar(const char* name, std::function<void()> func) {
    tests().push_back({name, func});
  }
};

#define TEST(name) \
  static void name(); \
  static TestRegistrar registrar_##name(#name, name); \
  static void name()

static void report_failure(const char* file, int line, const std::string& message) {
  ++g_failures;
  printf("\n  FAIL %s:%d\n    %s\n", file, line, message.c_str());
}

#define CHECK(cond) do { \
    ++g_checks; \
    if (!(cond)) report_failure(__FILE__, __LINE__, #cond); \
  } while (0)

#define CHECK_EQ_STR(actual, expected) do { \
    ++g_checks; \
    std::string a_ = (actual); \
    std::string e_ = (expected); \
    if (a_ != e_) report_failure(__FILE__, __LINE__, \
      std::string(#actual) + "\n    expected: \"" + e_ + "\"\n    actual:   \"" + a_ + "\""); \
  } while (0)

#define CHECK_EQ_INT(actual, expected) do { \
    ++g_checks; \
    long long a_ = (long long)(actual); \
    long long e_ = (long long)(expected); \
    if (a_ != e_) report_failure(__FILE__, __LINE__, \
      std::string(#actual) + "\n    expected: " + std::to_string(e_) + "\n    actual:   " + std::to_string(a_)); \
  } while (0)


// ---------------------------------------------------------------------------
// helpers

static YamlNodeArray parse(const char* text) {
  return YamlNodeArray::fromBytes(text, strlen(text));
}

// Structural dump that shows every node's key and type, unlike toString().
static void dump(const YamlNodeArray& nodes, std::string& out, int depth) {
  for (const auto& node : nodes) {
    out += std::string(depth * 2, ' ');
    out += node.key;
    if (node.isString()) {
      out += " = \"" + node.asString() + "\"\n";
    }
    else if (node.isObject()) {
      out += ":\n";
      dump(node.asObjects(), out, depth + 1);
    }
    else {
      out += " (type " + std::to_string(node.getType()) + ")\n";
    }
  }
}

static std::string dump(const YamlNodeArray& yaml) {
  if (yaml.isError()) {
    return std::string("ERROR: ") + yaml.getLastError() + "\n";
  }
  std::string out;
  dump(yaml, out, 0);
  return out;
}

static size_t child_count(const YamlNodeArray& yaml, const char* keys) {
  auto node = yaml.findKeys(keys);
  return (node != nullptr && node->isObject()) ? node->asObjects().size() : (size_t)-1;
}


// ---------------------------------------------------------------------------
// YAML: basic structure

TEST(yaml_flat_map) {
  auto yaml = parse("a: 1\nb: two\n");
  CHECK(!yaml.isError());
  CHECK_EQ_STR(yaml.getString("a"), "1");
  CHECK_EQ_STR(yaml.getString("b"), "two");
  CHECK_EQ_INT(yaml.getInt("a"), 1);
  CHECK_EQ_STR(yaml.getString("missing", "def"), "def");
}

TEST(yaml_nested_map) {
  auto yaml = parse(
    "a:\n"
    "  b: 1\n"
    "  c:\n"
    "    d: 2\n"
    "  e: 3\n"
    "f: 4\n");
  CHECK(!yaml.isError());
  CHECK_EQ_STR(yaml.getString("a:b"), "1");
  CHECK_EQ_STR(yaml.getString("a:c:d"), "2");
  CHECK_EQ_STR(yaml.getString("a:e"), "3");
  CHECK_EQ_STR(yaml.getString("f"), "4");
  CHECK_EQ_INT(yaml.size(), 2);
}

TEST(yaml_list_of_multiline_objects) {
  auto yaml = parse(
    "emotions:\n"
    "  - name: a\n"
    "    eyes: eyes_a\n"
    "  - name: b\n"
    "    eyes: eyes_b\n"
    "    nose: nose_b\n"
    "version: 1\n");
  CHECK(!yaml.isError());
  CHECK_EQ_INT(child_count(yaml, "emotions"), 2);
  CHECK_EQ_STR(yaml.getString("emotions:0:name"), "a");
  CHECK_EQ_STR(yaml.getString("emotions:0:eyes"), "eyes_a");
  CHECK_EQ_STR(yaml.getString("emotions:1:name"), "b");
  CHECK_EQ_STR(yaml.getString("emotions:1:nose"), "nose_b");
  CHECK_EQ_STR(yaml.getString("version"), "1");
}

TEST(yaml_list_of_scalars) {
  auto yaml = parse(
    "images:\n"
    "  - png/a.png\n"
    "  - png/b.png\n"
    "next: 1\n");
  CHECK(!yaml.isError());
  CHECK_EQ_INT(child_count(yaml, "images"), 2);
  CHECK_EQ_STR(yaml.getString("images:0"), "png/a.png");
  CHECK_EQ_STR(yaml.getString("images:1"), "png/b.png");
  CHECK_EQ_STR(yaml.getString("next"), "1");
}

TEST(yaml_sequence_lines_with_commas) {
  auto yaml = parse(
    "sequences:\n"
    "  - draw, 0, eyes, mirror, 0, 0, 1.0\n"
    "  - none, 0.5\n");
  CHECK(!yaml.isError());
  CHECK_EQ_STR(yaml.getString("sequences:0"), "draw, 0, eyes, mirror, 0, 0, 1.0");
  CHECK_EQ_STR(yaml.getString("sequences:1"), "none, 0.5");
}

TEST(yaml_quotes_and_comments) {
  auto yaml = parse(
    "# leading comment\n"
    "c: '#00ff00'\n"
    "d: x # trailing comment\n"
    "e: \"quoted value\"\n"
    "\n"
    "  # indented comment\n"
    "f: 1\n");
  CHECK(!yaml.isError());
  CHECK_EQ_STR(yaml.getString("c"), "#00ff00");
  CHECK_EQ_STR(yaml.getString("d"), "x");
  CHECK_EQ_STR(yaml.getString("e"), "quoted value");
  CHECK_EQ_STR(yaml.getString("f"), "1");
}

TEST(yaml_unquoted_hash_is_comment) {
  auto yaml = parse("color: #00ff00\n");
  CHECK(!yaml.isError());
  CHECK_EQ_STR(yaml.getString("color", "def"), "");
}

TEST(yaml_crlf_line_endings) {
  auto yaml = parse("a:\r\n  b: 1\r\nc: 2\r\n");
  CHECK(!yaml.isError());
  CHECK_EQ_STR(yaml.getString("a:b"), "1");
  CHECK_EQ_STR(yaml.getString("c"), "2");
}

TEST(yaml_object_key_is_not_a_string) {
  auto yaml = parse("a:\n  b: 1\n");
  CHECK_EQ_STR(yaml.getString("a", "def"), "def");
  CHECK(yaml.isKeysExist("a"));
  CHECK(!yaml.isKeysExist("a:b:c"));
}


// ---------------------------------------------------------------------------
// YAML: C2 cases (stack and indentation mismatch)

// C2-1: a line with a value cannot have children.
TEST(yaml_c2_1_children_under_value_is_error) {
  auto yaml = parse(
    "a: 1\n"
    "  b: 2\n"
    "c: 3\n");
  CHECK(yaml.isError());
}

// C2-2: consecutive single-line list items are siblings, not nested.
TEST(yaml_c2_2_single_line_items_are_siblings) {
  auto yaml = parse(
    "list:\n"
    "  - name: a\n"
    "  - name: b\n"
    "  - name: c\n");
  CHECK(!yaml.isError());
  CHECK_EQ_INT(child_count(yaml, "list"), 3);
  CHECK_EQ_INT(child_count(yaml, "list:0"), 1);
  CHECK_EQ_STR(yaml.getString("list:0:name"), "a");
  CHECK_EQ_STR(yaml.getString("list:1:name"), "b");
  CHECK_EQ_STR(yaml.getString("list:2:name"), "c");
}

// C2-3: a shallower line after a single-line list item returns to the right level.
TEST(yaml_c2_3_return_after_single_line_item) {
  auto yaml = parse(
    "outer:\n"
    "  list:\n"
    "    - name: a\n"
    "  after: 1\n"
    "root: 2\n");
  CHECK(!yaml.isError());
  CHECK_EQ_INT(child_count(yaml, "outer:list"), 1);
  CHECK_EQ_INT(child_count(yaml, "outer:list:0"), 1);
  CHECK_EQ_STR(yaml.getString("outer:after"), "1");
  CHECK_EQ_STR(yaml.getString("root"), "2");
  CHECK_EQ_INT(yaml.size(), 2);
}

// C2-4: a key whose children are all commented out stays empty.
TEST(yaml_c2_4_empty_key_then_sibling) {
  auto yaml = parse(
    "parent:\n"
    "  # child: 1\n"
    "sibling: 2\n"
    "nested:\n"
    "  empty:\n"
    "  value: 3\n");
  CHECK(!yaml.isError());
  CHECK_EQ_INT(child_count(yaml, "parent"), 0);
  CHECK_EQ_STR(yaml.getString("sibling"), "2");
  CHECK_EQ_INT(child_count(yaml, "nested:empty"), 0);
  CHECK_EQ_STR(yaml.getString("nested:value"), "3");
  CHECK_EQ_INT(yaml.size(), 3);
}

// Several shallower levels at once, the case that popped the root in the old parser.
TEST(yaml_return_several_levels) {
  auto yaml = parse(
    "a:\n"
    "  b:\n"
    "    c:\n"
    "      d: 1\n"
    "e: 2\n");
  CHECK(!yaml.isError());
  CHECK_EQ_STR(yaml.getString("a:b:c:d"), "1");
  CHECK_EQ_STR(yaml.getString("e"), "2");
}

TEST(yaml_list_item_with_nested_object) {
  auto yaml = parse(
    "list:\n"
    "  - name: a\n"
    "    sub:\n"
    "      x: 1\n"
    "    after: 2\n"
    "  - name: b\n");
  CHECK(!yaml.isError());
  CHECK_EQ_STR(yaml.getString("list:0:sub:x"), "1");
  CHECK_EQ_STR(yaml.getString("list:0:after"), "2");
  CHECK_EQ_STR(yaml.getString("list:1:name"), "b");
}

TEST(yaml_list_item_opening_object) {
  auto yaml = parse(
    "list:\n"
    "  - video:\n"
    "      path: a.gif\n"
    "    loop: true\n"
    "  - name: b\n");
  CHECK(!yaml.isError());
  CHECK_EQ_INT(child_count(yaml, "list"), 2);
  CHECK_EQ_STR(yaml.getString("list:0:video:path"), "a.gif");
  CHECK_EQ_STR(yaml.getString("list:0:loop"), "true");
  CHECK_EQ_STR(yaml.getString("list:1:name"), "b");
}

TEST(yaml_indent_errors) {
  CHECK(parse("a:\n   b: 1\n").isError());       // odd indent
  CHECK(parse("a:\n    b: 1\n").isError());      // skipped a level
  CHECK(parse("a:\n  b: 1\n    c: 2\n").isError()); // children under a value
  CHECK(parse("list:\n  - name: a\n      x: 1\n").isError()); // item continuation too deep
  CHECK(parse("list:\n  - a\n    x: 1\n").isError()); // children under a scalar item
}

TEST(yaml_unindented_list_is_error) {
  // Standard YAML allows this, but the parser requires list items to be indented under their key.
  auto yaml = parse(
    "list:\n"
    "- a\n"
    "- b\n");
  CHECK(yaml.isError());
}

TEST(yaml_format_error) {
  CHECK(parse("just text\n").isError());
}

TEST(yaml_empty_input) {
  auto yaml = parse("");
  CHECK(!yaml.isError());
  CHECK_EQ_INT(yaml.size(), 0);
}


// ---------------------------------------------------------------------------
// strutil

TEST(strutil_parse_time_str) {
  CHECK_EQ_INT(parse_time_str("10ms"), 10);
  CHECK_EQ_INT(parse_time_str("1.5s"), 1500);
  CHECK_EQ_INT(parse_time_str("2min"), 120000);
  CHECK_EQ_INT(parse_time_str("1h"), 3600000);
  CHECK_EQ_INT(parse_time_str("1d"), 86400000);
  CHECK_EQ_INT(parse_time_str("10"), 0);    // no unit
  CHECK_EQ_INT(parse_time_str("10 ms"), 0); // unit must follow directly
  CHECK_EQ_INT(parse_time_str(""), 0);
}

TEST(strutil_parse_hex) {
  CHECK_EQ_INT(parse_hex("0x3C"), 0x3C);
  CHECK_EQ_INT(parse_hex("0xff"), 0xff);
  CHECK_EQ_INT(parse_hex("#123456"), 0x563412); // stored as 0xBBGGRR
  CHECK_EQ_INT(parse_hex("#12345"), 0);
  CHECK_EQ_INT(parse_hex(""), 0);
}

TEST(strutil_parse_bool) {
  CHECK(parse_bool("true"));
  CHECK(parse_bool("yes"));
  CHECK(parse_bool("on"));
  CHECK(parse_bool("enable"));
  CHECK(parse_bool("1"));
  CHECK(!parse_bool("false"));
  CHECK(!parse_bool("0"));
  CHECK(!parse_bool(""));
}

TEST(strutil_parse_mac) {
  uint8_t mac[6] = {0};
  CHECK(parse_mac("12:34:56:78:9a:BC", mac));
  CHECK_EQ_INT(mac[0], 0x12);
  CHECK_EQ_INT(mac[5], 0xBC);
  CHECK(!parse_mac("12:34:56:78:9a", mac));
  CHECK(!parse_mac("12-34-56-78-9a-bc", mac));
}

TEST(strutil_my_split) {
  auto parts = my_split("draw, 0 ,eyes,,  mirror", ",");
  CHECK_EQ_INT(parts.size(), 4);
  CHECK_EQ_STR(parts[0], "draw");
  CHECK_EQ_STR(parts[1], "0");
  CHECK_EQ_STR(parts[2], "eyes");
  CHECK_EQ_STR(parts[3], "mirror");
}

TEST(strutil_parse_separate) {
  std::string left, right;
  CHECK(parse_separate("key=value", "=", left, right));
  CHECK_EQ_STR(left, "key");
  CHECK_EQ_STR(right, "value");
  CHECK(!parse_separate("novalue", "=", left, right));
}

TEST(strutil_format_frame_path) {
  char out[32];
  CHECK(format_frame_path(out, sizeof(out), "png/%03d.png", 7));
  CHECK_EQ_STR(out, "png/007.png");
  CHECK(format_frame_path(out, sizeof(out), "%d.png", 12));
  CHECK_EQ_STR(out, "12.png");
  CHECK(format_frame_path(out, sizeof(out), "100%%/%d", 1));
  CHECK_EQ_STR(out, "100%/1");
  CHECK(!format_frame_path(out, sizeof(out), "%d_%d.png", 1)); // two conversions
  CHECK(!format_frame_path(out, sizeof(out), "%s.png", 1));    // not an integer
  CHECK(!format_frame_path(out, 4, "%d.png", 12345));          // does not fit
}

TEST(strutil_appendf) {
  char buf[8];
  size_t len = 0;
  buf[0] = 0;
  appendf(buf, sizeof(buf), len, "%s", "abc");
  CHECK_EQ_STR(buf, "abc");
  CHECK_EQ_INT(len, 3);
  appendf(buf, sizeof(buf), len, "%d", 12345);
  CHECK_EQ_STR(buf, "abc1234");
  CHECK_EQ_INT(len, 7);
  appendf(buf, sizeof(buf), len, "more");
  CHECK_EQ_STR(buf, "abc1234");
  CHECK_EQ_INT(len, 7);
}


// ---------------------------------------------------------------------------

int main(int argc, char** argv) {
  // unbuffered, so the last test name is visible if a test crashes
  setvbuf(stdout, nullptr, _IONBF, 0);

  if (argc == 3 && strcmp(argv[1], "--dump") == 0) {
    printf("%s", dump(YamlNodeArray::fromFile(argv[2])).c_str());
    return 0;
  }

  const char* filter = (argc == 2) ? argv[1] : nullptr;

  for (const auto& test : tests()) {
    if (filter != nullptr && strstr(test.name, filter) == nullptr) {
      continue;
    }
    g_current_test = test.name;
    int before = g_failures;
    printf("%-50s", test.name);
    test.func();
    printf((g_failures == before) ? "ok\n" : "FAILED\n");
  }

  printf("\n%d checks, %d failures\n", g_checks, g_failures);
  return (g_failures == 0) ? 0 : 1;
}
