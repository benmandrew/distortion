#include <iostream>
#include "main.h"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cout << argv[0] << " Version "
            << Distortion_VERSION_MAJOR << "."
            << Distortion_VERSION_MINOR << std::endl;
        return 1;
    }
    const double v = std::stod(argv[1]);
    std::cout << "Hello world! " << v << std::endl;
    return 0;
}
