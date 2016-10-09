#include <lick/lick.h>
#include <raspi-phone-tools/file.h>

FIXTURE(ok) {
  auto nice = phone::file_t::make("/dev/null", 0);
}