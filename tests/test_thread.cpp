#include "../fatdog/thread.h"
#include "../fatdog/log.h"
#include <vector>
#include <unistd.h>

fatdog::Logger::ptr g_logger = FATDOG_LOG_ROOT();

int count = 0;
// fatdog::RWMutex s_mutex;
fatdog::Mutex s_mutex;

void fun1()
{
    FATDOG_LOG_INFO(g_logger) << "name: " << fatdog::Thread::GetName()
                              << " this.name: " << fatdog::Thread::GetThis()->getName()
                              << " id: " << fatdog::GetThreadId()
                              << " this.id: " << fatdog::Thread::GetThis()->getID();

    for (int i = 0; i < 10000; ++i)
    {
        // fatdog::RWMutex::WriteLock lock(s_mutex);
        fatdog::Mutex::Lock lock(s_mutex);
        ++count;
    }
}

void fun2()
{
    while (true)
    {
        FATDOG_LOG_INFO(g_logger) << "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
    }
}

void fun3()
{
    while (true)
    {
        FATDOG_LOG_INFO(g_logger) << "========================================";
    }
}

int main(int argc, char **argv)
{
    FATDOG_LOG_INFO(g_logger) << "thread test begin";
    std::vector<fatdog::Thread::ptr> thrs;
    for (int i = 0; i < 2; ++i)
    {
        fatdog::Thread::ptr thr(new fatdog::Thread("name_" + std::to_string(i * 2), &fun2));
        fatdog::Thread::ptr thr2(new fatdog::Thread("name_" + std::to_string(i * 2 + 1), &fun3));
        thrs.push_back(thr);
        thrs.push_back(thr2);
    }

    for (int i = 0; i < thrs.size(); ++i)
    {
        thrs[i]->join();
    }
    FATDOG_LOG_INFO(g_logger) << "thread test end";
    FATDOG_LOG_INFO(g_logger) << "count=" << count;

    return 0;
}
