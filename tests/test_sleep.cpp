#include <dlfcn.h>
#include <unistd.h>

#include <iostream>

extern "C"
{
    typedef unsigned int (*sleep_fun)(unsigned int seconds);
    sleep_fun sleep_f = (sleep_fun)dlsym(RTLD_NEXT, "sleep");

unsigned int sleep(unsigned int seconds) {
    std::cout << "hehe" << std::endl;
    sleep_f(seconds);
    return 0;
}
}

int main()
{
    int i = 10;
    while (i--)
    {
        sleep(1);
    }
    
    return 0;
}