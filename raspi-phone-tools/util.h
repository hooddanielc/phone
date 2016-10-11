#pragma once
#include <string>
#include <sys/types.h>
#include <unistd.h>
#include <vector>
#include <string.h>
#include <iostream>

#ifdef __APPLE__
  #include <mach-o/dyld.h>
#endif

namespace phone {
  std::string exec_path();
  std::string join_path(const std::vector<std::string> &parts);
  std::vector<std::string> split(const char *s, const char *c);
  std::vector<std::string> split_path(const char *s);
  std::string dirname();
  bool path_exists(const char *path);
  void unlink(const char *path);
}
