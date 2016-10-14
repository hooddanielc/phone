#include <lick/lick.h>
#include <raspi-phone-tools/util.h>
#include <iostream>

FIXTURE(canonicalize_path) {
  std::string path1 = util::canonicalize("/tmp/ok/../ok/..");
  std::string path2 = util::canonicalize("/tmp/ok/ok/ok/../..");
  std::string expected = util::canonicalize("/tmp/ok");
  EXPECT_EQ(expected, path1);
  EXPECT_EQ(expected, path2);
}

FIXTURE(cwd_no_except) {
  util::get_cwd();
}

FIXTURE(join_path) {
  util::join_path({
    "tmp",
    "ok"
  });

  EXPECT_EQ("/tmp/ok", util::join_path({
    "tmp",
    "ok"
  }, true));
}

FIXTURE(open_file) {
  util::fd_t file = util::open(util::join_path({
    "/tmp",
    "tmp.txt"
  }), util::access_t::read_write, util::if_not_exists_t(0700));


  EXPECT_EQ(file.path(), util::canonicalize("/tmp/tmp.txt"));
  EXPECT_EQ(util::exists("/tmp/tmp.txt"), true);
  util::unlink("/tmp/tmp.txt");
  EXPECT_EQ(util::exists("/tmp/tmp.txt"), false);
}

FIXTURE(open_unique) {
  auto file1 = util::open_unique("/tmp");
  auto file2 = util::open_unique("/tmp");
  auto file3 = util::open_unique("/tmp");

  EXPECT_NE(file1.path(), file2.path());
  EXPECT_NE(file1.path(), file3.path());
  EXPECT_NE(file2.path(), file1.path());
  EXPECT_NE(file2.path(), file3.path());
  EXPECT_NE(file3.path(), file1.path());
  EXPECT_NE(file3.path(), file2.path());
  EXPECT_TRUE(util::exists(file1.path()));
  EXPECT_TRUE(util::exists(file2.path()));
  EXPECT_TRUE(util::exists(file3.path()));
  util::unlink(file1.path());
  util::unlink(file2.path());
  util::unlink(file3.path());
  EXPECT_FALSE(util::exists(file1.path()));
  EXPECT_FALSE(util::exists(file2.path()));
  EXPECT_FALSE(util::exists(file3.path()));
}

FIXTURE(fd_t_unlink) {
  auto file1 = util::open_unique("/tmp");
  file1.unlink();
  EXPECT_FALSE(util::exists(file1.path()));
}

FIXTURE(write_and_read_exactly) {
  auto file1 = util::open_unique("/tmp");
  const char tmp[] = "The brown fox jumped over the fence";
  const int len = strlen(tmp);
  util::write_exactly(file1, tmp, len);
  char tmpread[len];
  lseek(file1, 0, SEEK_SET);
  util::read_exactly(file1, tmpread, len);
  tmpread[len] = '\0';
  EXPECT_EQ(std::string(tmp), std::string(tmpread));
  file1.unlink();
}

FIXTURE(write_dev_null_no_except) {
  auto file1 = util::make_fd_tty("/dev/null");
  const char tmp[] = "LOL IM SO COOL";
  const int len = strlen(tmp);
  util::write_exactly(file1, tmp, len);
}
