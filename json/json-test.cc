#include <lick/lick.h>
#include <json/json.h>
#include <sstream>
#include <iostream>

FIXTURE(object_constructs) {
  json_t::object_t a = {
    { "hash", "map" },
    { "number", 3 }
  };

  EXPECT_EQ(a["hash"], "map");
}

FIXTURE(array_constructs) {
  json_t::array_t a = {
    1,
    "one",
    "two",
    "three"
  };

  EXPECT_EQ(a[0], 1);
  EXPECT_EQ(a[1], "one");
  EXPECT_EQ(a[2], "two");
  EXPECT_EQ(a[3], "three");
}

FIXTURE(boolean_constructs) {
  json_t::boolean_t a = true;
  json_t::boolean_t b = false;

  EXPECT_EQ(a, true);
  EXPECT_EQ(b, false);
}

FIXTURE(number_constructs) {
  json_t::number_t a = 1;
  json_t::number_t b = 1.1;
  json_t::number_t c = 3.14;

  EXPECT_EQ(a, 1);
  EXPECT_EQ(b, 1.1);
  EXPECT_EQ(c, 3.14);
}

FIXTURE(string_constructs) {
  json_t::string_t a = "im a string";
  EXPECT_EQ(a, "im a string");
}

FIXTURE(casts_to_string) {
  json_t::object_t a = {
    { "a", "a" },
    { "b", 1 },
    { "c", { 1, 2, 3 }},
    { "d", json_t::object_t({{ "a", 1 }, { "b", 2 }}) },
    { "e", json_t::null_t() }
  };

  std::ostringstream str;
  str << a;
  EXPECT_EQ("{ \"a\": \"a\", \"b\": 1, \"c\": [ 1, 2, 3 ], \"d\": { \"a\": 1, \"b\": 2 }, \"e\": null }", str.str());
}

FIXTURE(json_object_from_string) {
  json_t a = json_t::decode("{ \"a\": \"a\", \"b\": 1, \"c\": [ 1, 2, 3 ], \"d\": { \"a\": 1, \"b\": 2 }, \"e\": null }");
  EXPECT_EQ(a["a"], "a");
  EXPECT_EQ(a["b"], 1);
  EXPECT_EQ(a["c"][0], 1);
  EXPECT_EQ(a["c"][1], 2);
  EXPECT_EQ(a["c"][2], 3);
  EXPECT_EQ(a["d"]["a"], 1);
  EXPECT_EQ(a["d"]["b"], 2);
  EXPECT_EQ(a["e"], json_t::null);
}
