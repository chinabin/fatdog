#include "util.h"

#include <unistd.h>
#include <sys/syscall.h>

#include "log.h"

namespace fatdog
{
    fatdog::Logger::ptr g_logger = FATDOG_LOG_NAME("system");

    pid_t GetThreadId()
    {
        return syscall(SYS_gettid);
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
} // namespace fatdog