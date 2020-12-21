#include "../fatdog/scheduler.h"
#include "../fatdog/log.h"
#include "../fatdog/macro.h"

static fatdog::Logger::ptr g_logger = FATDOG_LOG_ROOT();

void f()
{
    FATDOG_LOG_INFO(g_logger) << "in f()";
}

void test1_1()
{
    fatdog::Scheduler sc("AAA", true, 1);
    FATDOG_ASSERT(fatdog::Scheduler::GetThis() == &sc);
    sc.start();
    sc.schedule(f);
    sc.stop();
}

int main()
{
    test1_1();
}