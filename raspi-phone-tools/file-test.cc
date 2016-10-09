#include <lick/lick.h>
#include <raspi-phone-tools/file.h>
#include <raspi-phone-tools/util.h>
#include <iostream>

FIXTURE(current_executable) {
  EXPECT_EQ(phone::exec_path(), "/home/pi/out/debug/raspi-phone-tools/file-test");
}

FIXTURE(split_path_result) {
  auto result = phone::split("/a1/b2/c3", "/");
  EXPECT_EQ(result[1], "a1");
  EXPECT_EQ(result[2], "b2");
  EXPECT_EQ(result[3], "c3");
}

FIXTURE(split_path_result_2) {
  auto result = phone::split_path("/a1/b2/c3");

  EXPECT_EQ(result[1], "a1");
  EXPECT_EQ(result[2], "b2");
  EXPECT_EQ(result[3], "c3");
}

// Expects the out directory to be located
// in /home/pi
FIXTURE(current_executable_path) {
  auto result = phone::exec_path();
  EXPECT_EQ(result, "/home/pi/out/debug/raspi-phone-tools/file-test");
}

// Expects the out directory to be located
// in /home/pi
FIXTURE(dirname) {
  auto result = phone::dirname();
  EXPECT_EQ(result, "/home/pi/out/debug/raspi-phone-tools");
}

FIXTURE(join_path) {
  auto result = phone::join_path({
    "a1",
    "b2",
    "c3",
    "d4"
  });

  EXPECT_EQ(result, "a1/b2/c3/d4");
}
