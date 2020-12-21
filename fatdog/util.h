#ifndef __FATDOG_UTIL_H__
#define __FATDOG_UTIL_H__

#include <sys/types.h>
#include <execinfo.h>
#include <vector>
#include <string>

namespace fatdog
{
    pid_t GetThreadId();
    uint32_t GetFiberId();

    void Backtrace(std::vector<std::string> &bt, int size = 64, int skip = 1);
    std::string BacktraceToString(int size = 64, int skip = 2, const std::string &prefix = "");

    uint64_t GetCurrentMS();
    uint64_t GetCurrentUS();
} // namespace fatdog

#endif