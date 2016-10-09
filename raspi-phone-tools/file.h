#pragma once
#include <errno.h>
#include <fcntl.h> 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <memory>

namespace phone {
  class file_t {
  public:
    int fd;
    explicit file_t(int new_fd) noexcept;

    file_t(const file_t &) = delete;
    file_t &operator=(const file_t &) = delete;
    ~file_t();

    // size_t read(char *buf, size_t size);
    // size_t write(const char *buf, size_t size);
    static std::unique_ptr<file_t> make(const char *path, int flags);
    void set_interface_attribs(int speed, int parity);
    void set_blocking (int should_block);
  };
}
