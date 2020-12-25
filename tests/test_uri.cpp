#include "../fatdog/uri.h"
#include <iostream>

int main(int argc, char** argv) {
    //fatdog::Uri::ptr uri = fatdog::Uri::Create("http://www.fatdog.top/test/uri?id=100&name=fatdog#frg");
    //fatdog::Uri::ptr uri = fatdog::Uri::Create("http://admin@www.fatdog.top/test/中文/uri?id=100&name=fatdog&vv=中文#frg中文");
    fatdog::Uri::ptr uri = fatdog::Uri::Create("http://admin@www.fatdog.top");
    //fatdog::Uri::ptr uri = fatdog::Uri::Create("http://www.fatdog.top/test/uri");
    std::cout << uri->toString() << std::endl;
    auto addr = uri->createAddress();
    std::cout << *addr << std::endl;
    return 0;
}
