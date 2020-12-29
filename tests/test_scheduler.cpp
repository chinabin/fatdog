#include "../fatdog/scheduler.h"
#include "../fatdog/log.h"
#include "../fatdog/macro.h"
#include <unistd.h>

static fatdog::Logger::ptr g_logger = FATDOG_LOG_ROOT();
pid_t main_thread_id = -1;

void f()
{
    static int s_count = 5;
    FATDOG_LOG_INFO(g_logger) << "test in fiber s_count=" << s_count;

    //sleep(1);
    if(--s_count >= 0) {
        fatdog::Scheduler::GetThis()->schedule(&f, main_thread_id);
    }
}

int main()
{
    // main_thread_id = fatdog::GetThreadId();
    FATDOG_LOG_INFO(g_logger) << "main";
    fatdog::Scheduler sc("AAA", 1, true);
    FATDOG_ASSERT(fatdog::Scheduler::GetThis() == &sc);
    sc.start();
    sleep(1);
    FATDOG_LOG_INFO(g_logger) << "schedule";
    sc.schedule(f);
    sc.stop();
    sc.stop();
    FATDOG_LOG_INFO(g_logger) << "over";
}
