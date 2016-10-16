#include <string>
#include <iostream>
#include <thread>
#include <atomic>
#include <mutex>
#include <raspi-phone-tools/util.h>
#include <vector>
#include <utility>
#include <functional>
#include <json/json.h>

namespace phone {
  class phone_t {
    public:
      enum class event_t {
        reply,
        onchar,
        error,
        sms,
        call,
        missedcall
      };

      util::fd_t device;
      std::atomic<bool> run;
      std::vector<std::thread> tasks;
      phone_t(const char *portname);
      using callback_t = std::function<void(json_t::object_t)>;
      std::vector<std::pair<event_t, callback_t>> listeners;
      void on(event_t event, callback_t callback);
      void listen();
      void write(const std::string &msg);
      std::string read(size_t count);
      std::string read_to_nl();
      char read_char();
      int repl();
      void join();
      ~phone_t();
  };
}
