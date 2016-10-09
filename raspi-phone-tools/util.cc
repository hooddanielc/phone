#include <raspi-phone-tools/util.h>

namespace phone {
  std::string exec_path() {
    char result[PATH_MAX];
    size_t count = readlink("/proc/self/exe", result, PATH_MAX);
    return std::string{ result, count };
  }

  std::vector<std::string> split(const char *s, const char *c) {
    std::string str(s);
    std::vector<std::string> result;

    std::string tmp = "";
    for (char &it : str) {
      if (it == c[0]) {
        result.push_back(tmp);
        tmp = "";
        continue;
      } else {
        tmp += it;
      }
    }

    if (tmp != "") {
      result.push_back(tmp);
    }

    return result;
  }

  std::vector<std::string> split_path(const char *path) {
    return split(path, "/");
  }

  std::string join_path(const std::vector<std::string> &parts) {
    std::string result = "";
    auto iter = 0;
    for (auto it = parts.begin(); it != parts.end(); ++it) {
      if (iter != 0) {
        result += "/" + *it;
      } else {
        result += *it;
      }

      ++iter;
    }

    return result;
  }

  std::string dirname() {
    auto parts = split_path(exec_path().c_str());
    parts.erase(parts.end());
    return join_path(parts);
  }
}
