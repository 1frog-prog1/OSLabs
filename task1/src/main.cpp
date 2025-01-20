#include <func_lib/func.h>

#include <iostream>

int main(int argc, char** argv) {
    std::cout << "Hello, world!" << std::endl;

    if (argc >= 2) {
        FuncNS::func(argv[1]);
    }
}
