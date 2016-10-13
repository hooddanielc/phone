#include <raspi-phone-tools/util.h>

namespace util {

fd_t::fd_t(const fd_t &that) {
  handle = that.is_open()
      ? throw_if_lt0(dup(that.handle))
      : closed_handle;
}

std::string fd_t::path() {
  #ifdef __APPLE__
    char file_path[PATH_MAX];
    if (fcntl(get(), F_GETPATH, file_path) == -1) {
      throw_system_error();
    }
    return std::string{ file_path };
  #else
    char proclnk[0xFFF];
    char filename[0xFFF];
    sprintf(proclnk, "/proc/self/fd/%d", fileno((FILE*) get()));
    int r;
    r = readlink(proclnk, filename, PATH_MAX);
    filename[r] = '\0';

    if(r < 0) {
      throw_system_error();
    }

    return std::string{ filename };
  #endif
}

void fd_t::unlink() {
  util::unlink(path());
}

fd_t &fd_t::reset() noexcept {
  // Loop while we're still open.
  while (is_open()) {
    // Close our handle.  If that succeeds (which it nearly always does),
    // we can forget the handle and exit the loop.
    if (close(handle) == 0) {
      handle = closed_handle;
      break;
    }
    // Something went wrong while trying to close.
    switch (errno) {
      // The file descriptor was already closed?  Fine, we'll just go.
      case EBADF: {
        handle = closed_handle;
        break;
      }
      // We were interrupted by a signal.  Try again.
      case EINTR: {
        break;
      }
      // Some seriously process-ending thing happened.  Throwing an exception
      // here (in a noexcept function) will cause the process to exit.
      default: {
        throw_system_error();
      }
    }  // switch
  }  // while
  return *this;
}

///////////////////////////////////////////////////////////////////////////////

std::string canonicalize(const std::string &path) {
  char tmp[PATH_MAX];
  realpath(path.c_str(), tmp);
  return std::string{ tmp };
}

bool exists(const std::string &path) {
  struct stat st;
  if (stat(path.c_str(), &st) == 0) {
    return true;
  }
  if (errno != ENOENT) {
    throw_system_error();
  }
  return false;
}

std::string get_cwd() {
  char cwd[PATH_MAX];
  getcwd(cwd, PATH_MAX);
  return std::string{ cwd };
}

std::string join_path(const std::vector<std::string> &parts, bool absolute) {
  std::ostringstream strm;
  bool separated = false;
  for (const auto &part: parts) {
    if (part.empty()) {
      continue;
    }
    if (absolute && !separated && part.front() != path_sep) {
      strm << path_sep;
    }
    strm << part;
    separated = (part.back() == path_sep);
    absolute = true;
  }  // for
  return strm.str();
}

fd_t open(
    const std::string &path, access_t access,
    if_not_exists_t if_not_exists, if_exists_t if_exists) {
  int flags;
  mode_t mode;
  switch (access) {
    case access_t::read_only: {
      flags = O_RDONLY;
      break;
    }
    case access_t::write_only: {
      flags = O_WRONLY;
      break;
    }
    case access_t::read_write: {
      flags = O_RDWR;
      break;
    }
  }  // switch
  if (exists(path)) {
    switch (if_exists) {
      case if_exists_t::open: {
        break;
      }
      case if_exists_t::append: {
        flags |= O_APPEND;
        break;
      }
      case if_exists_t::truncate: {
        flags |= O_TRUNC;
        break;
      }
      case if_exists_t::fail: {
        throw_system_error(EEXIST);
      }
    }  // switch
    mode = 0;
  } else if (if_not_exists.create) {
    flags |= O_CREAT;
    mode = if_not_exists.mode;
  } else {
    throw_system_error(ENOENT);
  }
  return make_fd(::open(path.c_str(), flags, mode));
}

fd_t open_unique(const std::string &prefix) {
  static const std::string suffix = "/XXXXXX";
  auto temp = prefix + suffix;
  return make_fd(mkstemp(const_cast<char *>(temp.c_str())));
}

void read_exactly(int fd, void *data, size_t size) {
  // Make a cursor pointing at the start of the buffer.
  auto *csr = static_cast<char *>(data);
  // Loop while we still have bytes left to read.
  while (size) {
    // Read some bytes.  If we don't get any, we have reached an unexpected
    // end-of-file.
    auto actl = read_at_most(fd, csr, size);
    if (!actl) {
      throw_system_error(ENODATA);
    }
    // We read some bytes.  Bump the cursor ahead and reduce the number of
    // bytes left to read.
    csr += actl;
    size -= actl;
  }  // while
}

std::vector<std::string> split_path(const std::string &path, bool *absolute) {
  std::vector<std::string> result;
  const char *csr = path.data(), *limit = csr + path.size();
  if (csr < limit) {
    if (*csr == path_sep) {
      if (absolute) {
        *absolute = true;
      }
      ++csr;
    } else if (absolute) {
      *absolute = false;
    }
    const char *anchor = nullptr;
    enum { start, skip, accept } state = start;
    bool go = true;
    do {
      switch (state) {
        case start: {
          if (csr >= limit) {
            go = false;
          } else if (*csr == path_sep) {
            state = skip;
            ++csr;
          } else {
            anchor = csr;
            state = accept;
            ++csr;
          }
          break;
        }
        case skip: {
          if (csr < limit && *csr == path_sep) {
            ++csr;
          } else {
            state = start;
          }
          break;
        }
        case accept: {
          if (csr < limit && *csr != path_sep) {
            ++csr;
          } else {
            result.emplace_back(anchor, csr - anchor);
            anchor = nullptr;
            state = start;
          }
          break;
        }
      }  // switch
    } while (go);
  }
  return result;
}

std::string standardize_string(char *c_str) {
  std::string result;
  try {
    result = c_str;
  } catch (...) {
    free(c_str);
    throw;
  }
  free(c_str);
  return result;
}

void unlink(const std::string &path) {
  throw_if_lt0(::unlink(path.c_str()));
}

void write_exactly(int fd, const void *data, size_t size) {
  // Make a cursor pointing at the start of the buffer.
  auto *csr = static_cast<const char *>(data);
  // Loop while we still have bytes left to write.
  while (size) {
    // Write some bytes.  If can't, the thing must be full.
    auto actl = write_at_most(fd, csr, size);
    if (!actl) {
      throw_system_error(ENOSPC);
    }
    // We wrote some bytes.  Bump the cursor ahead and reduce the number of
    // bytes left to write.
    csr += actl;
    size -= actl;
  }  // while
}

[[noreturn]] void throw_system_error(int code) {
  throw std::system_error { code, std::system_category() };
}

#if defined(__WIN32__)
const char path_sep = '\\';
#else
const char path_sep = '/';
#endif

}  // util
