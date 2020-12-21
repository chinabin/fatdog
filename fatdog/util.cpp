#include "util.h"

#include <unistd.h>
#include <sys/syscall.h>
#include <sys/time.h>

#include "log.h"
#include "fiber.h"

namespace fatdog
{
    fatdog::Logger::ptr g_logger = FATDOG_LOG_NAME("system");

    pid_t GetThreadId()
    {
        return syscall(SYS_gettid);
    }

    uint32_t GetFiberId()
    {
        return fatdog::Fiber::GetFiberId();
    }

    void Backtrace(std::vector<std::string> &bt, int size, int skip)
    {
        void **buffer = (void **)malloc(sizeof(void *) * size);

        int nptrs = backtrace(buffer, size);
        char **strings = backtrace_symbols(buffer, nptrs);

        if (strings == NULL)
        {
            FATDOG_LOG_ERROR(g_logger) << "backtrace_synbols error";
        }

        for (int j = skip; j < nptrs; ++j)
        {
            bt.push_back(strings[j]);
        }

        free(buffer);
        free(strings);
    }

    std::string BacktraceToString(int size, int skip, const std::string &prefix)
    {
        std::vector<std::string> bt;
        Backtrace(bt, size, skip);
        std::stringstream ss;
        for (size_t i = 0; i < bt.size(); ++i)
        {
            ss << prefix << bt[i] << std::endl;
        }
        return ss.str();
    }

uint64_t GetCurrentMS()
{
    timeval tm;
    gettimeofday(&tm, nullptr);

    return tm.tv_sec * 1000 + tm.tv_usec / 1000;
}

uint64_t GetCurrentUS()
{
    timeval tm;
    gettimeofday(&tm, nullptr);

    return tm.tv_sec * 1000 * 1000 + tm.tv_usec;
}

} // namespace fatdog