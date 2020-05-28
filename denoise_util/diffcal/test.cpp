#include <libvmaf/libvmaf.h>
#include <iostream>

int main() {
    std::cout << "Hello world!" << std::endl;
   
    compute_vmaf(nullptr, nullptr, 0, 0, nullptr, nullptr, nullptr, nullptr, nullptr, 0, 0, 0, 0, 0, 0, 0, nullptr, 0, 0, 0);
}
