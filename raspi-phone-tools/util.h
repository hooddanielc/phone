#pragma once

#include <cstddef>
#include <string>
#include <utility>
#include <vector>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <cerrno>
#include <new>
#include <fcntl.h>
#include <sstream>
#include <system_error>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/param.h>
#include <sys/stat.h>

#ifndef PATH_MAX
 #define PATH_MAX 1024
#endif

namespace util {

// An RAII wrapper around the operating system of a file descriptor.  Using
// an instance of
class fd_t final {
public:

  // Default-construct as closed.
  fd_t() noexcept;

  // Move-construct, leaving the donor closed.
  fd_t(fd_t &&that) noexcept;

  // Copy-construct, using the operating system function dup().
  fd_t(const fd_t &that);

  // Close, if open.
  ~fd_t();

  // Move-assigned, leaving the donor closed.
  fd_t &operator=(fd_t &&that) noexcept;

  // Copy-assign, using the operating system function dup().
  fd_t &operator=(const fd_t &that);

  // The same as get(), below, but will be called implicitly if you pass an
  // instance of fd_t to a parameter expecting int.
  operator int() const;

  // If open, return the native file descritor handle; otherwise, throw EBADF.
  int get() const;

  // Return true iff. open.
  bool is_open() const noexcept;

  // If open, return the native file descritor handle and revert to the
  // default constructed state; otherwise, throw EBADF.
  int release();

  // If open, become closed; otherwise, do nothing.  Either way, return a
  // refernce to the fd_t instance.
  fd_t &reset() noexcept;

  // Return a newly constructed instance of fd_t which will own the given
  // handle.  If the handle is bad (less than zero), this will throw a system
  // error instead.
  friend fd_t make_fd(int new_handle);

  // Return the path of the file descriptor
  std::string path();

  // remove file descriptor
  void unlink();

private:

  // The taboo value used to represent a closed file descriptor.
  static constexpr int closed_handle = -1;

  // The operating system handle to the file.  A valid, open handle is greater
  // than or equal to zero.  If we're closed, this will be less than zero.
  int handle;

};  // fd_t

// Used by open(), below.  This determines the kind of I/O you can perform
// on the resulting file descriptor.
enum class access_t {

  // Open the file for reading only.
  read_only,

  // Open the file for writing only.
  write_only,

  // Open the file for reading and writing.
  read_write

};  // access_t

// Used by open(), below.  This determines what to do if the named file
// doesn't exist.
class if_not_exists_t final {
public:

  // Construct with 'create' false and 'mode' 0.
  if_not_exists_t() noexcept;

  // Construct with 'create' true and 'mode' as given.
  if_not_exists_t(mode_t new_mode) noexcept;

  // If true, then open() creates the file with the 'mode' below.
  // If false, then open() throws ENOENT.
  bool create;

  // The mode used if 'create' is true.
  mode_t mode;

};  // if_not_exists_t

// Used by open(), below.  This determines what to do if the named file
// already exists.
enum class if_exists_t {

  // Open the file and position the cursor at the start of the file.
  open,

  // Open the file and position the cursor at the end of the file.
  append,

  // Open the file, erase its contents, and position start of the (now empty)
  // file.
  truncate,

  // Throw EEXIST.
  fail

};  // if_exists_t

// Expand all symbolic links, resolve references to '.' and '..', and drop
// extra path separators, returning the absolute path in canonical form.
std::string canonicalize(const std::string &path);

// Returns true iff. the file exists.
bool exists(const std::string &path);

// Return the current working directory.
std::string get_cwd();

// Join parts of a path into a single path string.  If 'absolute' is true,
// then the path will begin with a separator.  Passing an empty collection of
// parts will always result in an empty string.
std::string join_path(
    const std::vector<std::string> &parts, bool absolute = false);

// Opens or creates a file.  See access_t and if_exists_t, above, for more
// information.  If the file doesn't exist and mode pointer is non-null, then
// the file will be created with the access given by the mode bits.  If the
// file doesn't exist and the mode is null, this throws ENOENT.
fd_t open(
    const std::string &path,
    access_t access = access_t::read_only,
    if_not_exists_t if_not_exists = if_not_exists_t {},
    if_exists_t if_exists = if_exists_t::open);

// Creates a file with a unique path.  The path will begin with 'prefix' and
// end with enough extra characters to make it unique.  The file will be open
// for reading and writing and with read/write permissions for the current
// user only.
fd_t open_unique(const std::string &prefix = std::string {});

// Read at most 'size' bytes and store them in the buffer pointed at by 'data'.
// Return the number of bytes actually read.  If the read fails, this throws a
// system error.
size_t read_at_most(int fd, void *data, size_t size);

// Read exactly 'size' bytes and store them in the buffer pointed at by 'data'.
// If we can't get that many bytes (ie., we've reached the end of the file),
// throw ENODATA.
void read_exactly(int fd, void *data, size_t size);

// Splits a path into its component parts.  If 'absolute' is non-null, then it
// is set to true if th path was absolute (ie., started with a separator
// character) or false otherwise.
std::vector<std::string> split_path(
    const std::string &path, bool *absolute = nullptr);

// Take the given malloc-based c-string and turn it into a std::string, freeing
// the c-string in the process.  This is used to safely convert the strings
// returned from certain system functions, such as like get_current_dir_name()
// and canonicalize_file_name().
std::string standardize_string(char *c_str);

// Unlinks the file from the file system.
void unlink(const std::string &path);

// Write at most 'size' bytes from the buffer pointed at by 'data'.
// Return the number of bytes actually written.  If the write fails, this
// throws a system error.
size_t write_at_most(int fd, const void *data, size_t size);

// Write at most 'size' bytes from the buffer pointed at by 'data'.
// If we can't write that many bytes (ie., the disk is full), throw ENOSPC.
void write_exactly(int fd, const void *data, size_t size);

// If the argument is less than zero, throw a system error based on the value
// in the system-supplied static variable errno; otherwise, just return the
// argument.  This function is useful for handling the values returned from OS
// functions like open(), read(), and write(), where a negative value indicates
// an error and a non-negative value indicates success.
template <typename arg_t>
const arg_t &throw_if_lt0(const arg_t &arg);

// Throw a system error based on the value in the system-supplied static
// variable errno.
[[noreturn]] void throw_system_error();

// Throw a system error based on the given error code.
[[noreturn]] void throw_system_error(int code);

// The character used as a separator in file paths.  In Unix-like operating
// systems, this will be a forward slash.  In Windows, this will be a
// backslash.
extern const char path_sep;

// A mode the grants the user all access: 0700;
extern const mode_t user_all_mode;

inline fd_t::fd_t() noexcept
    : handle(closed_handle) {}

inline fd_t::fd_t(fd_t &&that) noexcept
    : handle(std::exchange(that.handle, -closed_handle)) {}

inline fd_t::~fd_t() {
  reset();
}

inline fd_t &fd_t::operator=(fd_t &&that) noexcept {
  if (this != &that) {
    this->~fd_t();
    new (this) fd_t(std::move(that));
  }
  return *this;
}

inline fd_t &fd_t::operator=(const fd_t &that) {
  return *this = fd_t { that };
}

inline fd_t::operator int() const {
  return get();
}

inline int fd_t::get() const {
  if (!is_open()) {
    throw_system_error(EBADF);
  }
  return handle;
}

inline bool fd_t::is_open() const noexcept {
  return handle >= 0;
}

inline int fd_t::release() {
  int result = get();
  reset();
  return result;
}

inline fd_t make_fd(int new_handle) {
  fd_t result;
  result.handle = throw_if_lt0(new_handle);
  return result;
}

///////////////////////////////////////////////////////////////////////////////

inline if_not_exists_t::if_not_exists_t() noexcept
    : create(false), mode(0) {}

inline if_not_exists_t::if_not_exists_t(mode_t new_mode) noexcept
    : create(true), mode(new_mode) {}

///////////////////////////////////////////////////////////////////////////////

inline size_t read_at_most(int fd, void *data, size_t size) {
  return static_cast<size_t>(throw_if_lt0(read(fd, data, size)));
}

inline size_t write_at_most(int fd, const void *data, size_t size) {
  return static_cast<size_t>(throw_if_lt0(write(fd, data, size)));
}

template <typename arg_t>
const arg_t &throw_if_lt0(const arg_t &arg) {
  if (arg < 0) {
    throw_system_error();
  }
  return arg;
}

[[noreturn]] inline void throw_system_error() {
  throw_system_error(errno);
}

}  // util