#include <raspi-phone-tools/util.h>

namespace phone {
  std::string exec_path() {
    #ifdef __linux__
      char result[PATH_MAX];
      size_t count = readlink("/proc/self/exe", result, PATH_MAX);
      return std::string{ result, count };
    #endif

    #ifdef __APPLE__
      char path[PATH_MAX];
      uint32_t size = sizeof(path);

      if (_NSGetExecutablePath(path, &size) != 0) {
        throw "buffer too small for the executable path";
      }

      return std::string{ path };
    #endif
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
    parts.erase(parts.end() - 1);
    return join_path(parts);
  }

  bool path_exists(const char *path) {
    return access(path, F_OK) != -1;
  }

  void unlink(const char *path) {
    if (::unlink(path) == -1) {

    }
  }
}
