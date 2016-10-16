#include <raspi-phone-tools/phone.h>

void print_help() {
  std::cout << std::endl << "Usage" << std::endl << std::endl;
  std::cout << "phone-cli <port-name>" << std::endl << std::endl;
}

int main (int argc, char *argv[]) {
  if (argc < 2) {
    print_help();
    return 1;
  }

  if (argc > 2) {
    print_help();
    return 1;
  }

  std::string portname = argv[1];

  if (portname == "-h" || portname == "help") {
    print_help();
    return 0;
  } else {
    return phone::phone_t(portname.c_str()).repl();
  }

  return 0;
}
