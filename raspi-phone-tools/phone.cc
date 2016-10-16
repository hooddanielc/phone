#include <raspi-phone-tools/phone.h>

namespace phone {
  std::mutex rw_mutex;

  using callback_t = std::function<void(json_t::object_t)>;
  phone_t::phone_t(const char *portname) : device(util::make_fd_tty(portname)), run(true) {}
  phone_t::~phone_t() {}

  void phone_t::on(event_t event, callback_t callback) {
    listeners.push_back(std::pair<event_t, callback_t>(event, callback));
  }

  void phone_t::listen() {
    int dev = device;

    tasks.push_back(std::thread([&dev](std::atomic<bool> &run) {
      while (run.load()) {
        char buff[2];
        rw_mutex.lock();
        util::read_at_most(dev, buff, 1);
        rw_mutex.unlock();
        buff[1] = '\0';
        std::cout << "BUFF: `" << buff << "`" << std::endl;
      }
    }, std::ref(run)));
  }

  void phone_t::write(const std::string &msg) {
    rw_mutex.lock();
    util::write_exactly(device, msg.c_str(), msg.size());
    rw_mutex.unlock();
  }

  std::string phone_t::read(size_t count) {
    rw_mutex.lock();
    char buff[count];
    util::read_exactly(device, buff, count);
    rw_mutex.unlock();
    return std::string{ buff, count };
  }

  std::string phone_t::read_to_nl() {
    std::string result;
    char c = read_char();

    while (c != '\n') {
      result += c;
      c = read_char();
    }

    return result;
  }

  char phone_t::read_char() {
    rw_mutex.lock();
    char buff[1];
    util::read_exactly(device, buff, 1);
    rw_mutex.unlock();
    return buff[0];
  }

  int phone_t::repl() {
    std::string buffer;
    std::cin >> buffer;

    if (buffer == "q" || buffer == "quit") {
      return 0;
    } else {
      if (buffer.substr(0, 2) != "AT") {
        std::cout << "All commands must start with `AT`" << std::endl;
        return repl();
      }

      buffer += '\n';
      write(buffer);
      read_to_nl();
      std::cout << read_to_nl() << std::endl;
      return repl();
    }

    return 0;
  }

  // block and wait for everything to exit
  void phone_t::join() {
    for (auto it = tasks.begin(); it != tasks.end(); ++it) {
      (*it).join();
    }
  }
}
