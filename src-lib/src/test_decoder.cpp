#include <iostream>
#include "decoder.h"

int main() {
    // ✅ 直接调用函数（静态链接 .lib）
    std::cout << "DLL returned: " << hello_world() << std::endl;
    return 0;
}