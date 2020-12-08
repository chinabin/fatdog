#include "util.h"

#include <unistd.h>
#include <sys/syscall.h>

namespace fatdog
{
    pid_t GetThreadId()
    {
        return syscall(SYS_gettid);
    }
} // namespace fatdog