#include <raspi-phone-tools/file.h>

namespace phone {
  file_t::file_t(int new_fd) noexcept : fd(new_fd) {}

  file_t::~file_t() {
    close(fd);
  }

  std::unique_ptr<file_t> file_t::make(const char *path, int flags) {
    int new_fd = open(path, flags);
    return std::move(std::unique_ptr<file_t>(new file_t(new_fd)));
  }

  void file_t::set_interface_attribs(int speed, int parity) {
    struct termios tty;

    if (tcgetattr(fd, &tty) < 0) {
      printf("Error from tcgetattr: %s\n", strerror(errno));
      // throw and error! return -1;
    }

    cfsetospeed(&tty, (speed_t)speed);
    cfsetispeed(&tty, (speed_t)speed);

    tty.c_cflag |= (CLOCAL | CREAD);    /* ignore modem controls */
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;         /* 8-bit characters */
    tty.c_cflag &= ~PARENB;     /* no parity bit */
    tty.c_cflag &= ~CSTOPB;     /* only need 1 stop bit */
    tty.c_cflag &= ~CRTSCTS;    /* no hardware flowcontrol */

    /* setup for non-canonical mode */
    tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
    tty.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
    tty.c_oflag &= ~OPOST;

    /* fetch bytes as they become available */
    tty.c_cc[VMIN] = 1;
    tty.c_cc[VTIME] = 1;

    if (tcsetattr(fd, TCSANOW, &tty) != 0) {
      printf("Error from tcsetattr: %s\n", strerror(errno));
      // throw an error! return -1;
    }
  }

  void file_t::set_blocking (int should_block) {
    struct termios tty;
    memset (&tty, 0, sizeof tty);

    if (tcgetattr (fd, &tty) != 0) {
      // Throw an error! error_message ("error %d from tggetattr", errno);
      return;
    }

    tty.c_cc[VMIN]  = should_block ? 1 : 0;
    tty.c_cc[VTIME] = 5;

    if (tcsetattr (fd, TCSANOW, &tty) != 0) {
      // Throw an error! error_message ("error %d setting term attributes", errno);
    }
  }
}
