#pragma once


namespace phone {
  class modem_t {
    public:
      modem_t();
      ~modem_t();
      void begin(const char *port);

    private:
      int fd = 0;
  };
}
