#include "../fatdog/thread.h"
#include "../fatdog/log.h"
#include <vector>
#include <unistd.h>

fatdog::Logger::ptr g_logger = FATDOG_LOG_ROOT();

int count = 0;
fatdog::RWMutex s_mutex;

void fun1() {
    FATDOG_LOG_INFO(g_logger) << "name: " << fatdog::Thread::GetName()
                             << " this.name: " << fatdog::Thread::GetThis()->getName()
                             << " id: " << fatdog::GetThreadId()
                             << " this.id: " << fatdog::Thread::GetThis()->getID();

    for(int i = 0; i < 10000; ++i) {
        // fatdog::RWMutex::WriteLock lock(s_mutex);
        ++count;
    }
}

void fun2() {
}

int main(int argc, char** argv) {
    FATDOG_LOG_INFO(g_logger) << "thread test begin";
    std::vector<fatdog::Thread::ptr> thrs;
    for(int i = 0; i < 5; ++i) {
        fatdog::Thread::ptr thr(new fatdog::Thread("name_" + std::to_string(i), &fun1));
        thrs.push_back(thr);
    }

    for(int i = 0; i < 5; ++i) {
        thrs[i]->join();
    }
    FATDOG_LOG_INFO(g_logger) << "thread test end";
    FATDOG_LOG_INFO(g_logger) << "count=" << count;

    return 0;
}
