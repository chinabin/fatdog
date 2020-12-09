#ifndef __FATDOG_MACRO_H__
#define __FATDOG_MACRO_H__

#include <assert.h>
#include "util.h"
#include "log.h"

#define FATDOG_ASSERT(x)                                                                           \
    if (!(x))                                                                               \
    {                                                                                       \
        FATDOG_LOG_ERROR(FATDOG_LOG_ROOT()) << "ASSERTION: " << #x                          \
                                            << "\nbacktrace:\n"                             \
                                            << fatdog::BacktraceToString(100, 2, "      "); \
        assert(x);                                                                          \
    }

#define FATDOG_ASSERT2(x, s)                                                                       \
    if (!(x))                                                                               \
    {                                                                                       \
        FATDOG_LOG_ERROR(FATDOG_LOG_ROOT()) << "ASSERTION: " << #x                          \
                                            << "\n"                                         \
                                            << s                                            \
                                            << "\nbacktrace:\n"                             \
                                            << fatdog::BacktraceToString(100, 2, "      "); \
        assert(x);                                                                          \
    }

#endif