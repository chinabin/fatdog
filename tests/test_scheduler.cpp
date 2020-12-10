#include "../fatdog/scheduler.h"

int main()
{
    // fatdog::Scheduler::ptr s{new fatdog::Scheduler("wang")};
    // s->start();
    // s->stop();

    fatdog::Scheduler sc("wang");
    sc.start();
    sc.stop();
}