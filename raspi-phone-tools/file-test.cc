#include <lick/lick.h>
#include <raspi-phone-tools/file.h>
#include <raspi-phone-tools/util.h>
#include <iostream>

const std::string home(getenv("HOME"));

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
  EXPECT_EQ(phone::exec_path(), phone::join_path({
    home,
    "out",
    "debug",
    "raspi-phone-tools",
    "file-test"
  }));
}

// Expects the out directory to be located
// in /home/pi
FIXTURE(dirname) {
  EXPECT_EQ(phone::dirname(), phone::join_path({
    home,
    "out",
    "debug",
    "raspi-phone-tools"
  }));
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

// TODO UNLINK BE A NICE PEOPLE
FIXTURE(write_file_no_except) {
  auto path = phone::join_path({ phone::dirname(), "afile.txt" });

  {
    auto myfile = phone::file_t::make(path.c_str(), O_WRONLY | O_APPEND | O_CREAT, 0777);
    myfile->write("nice\n", 5);
    EXPECT_EQ(phone::path_exists(path.c_str()), true);
  }

  phone::unlink(path.c_str());
}

FIXTURE(read_file_no_except) {
  auto path = phone::join_path({ phone::dirname(), "testread.txt" });

  {
    auto myfile = phone::file_t::make(path.c_str(), O_WRONLY | O_APPEND | O_CREAT, 0777);
    myfile->write("nice\n", 5);
  }

  {
    auto myfile = phone::file_t::make(path.c_str(), O_RDONLY);
    char buffer[5];
    myfile->read(buffer, 5);
    buffer[4] = '\0';
    EXPECT_EQ("nice", std::string(buffer));
  }

  phone::unlink(path.c_str());
}

