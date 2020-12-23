#include "../fatdog/util.h"

#include "../fatdog/log.h"
#include "../fatdog/macro.h"
#include "../fatdog/fiber.h"
#include "../fatdog/thread.h"

#include <ucontext.h>
#include <unistd.h>

static fatdog::Logger::ptr g_logger = FATDOG_LOG_ROOT();

void run_in_fiber() {
    FATDOG_LOG_INFO(g_logger) << "run_in_fiber begin";
    fatdog::Fiber::YieldToHold();
    FATDOG_LOG_INFO(g_logger) << "run_in_fiber end";
    fatdog::Fiber::YieldToHold();
}

void test_fiber() {
    FATDOG_LOG_INFO(g_logger) << "main begin -1";
    {
        fatdog::Fiber::GetThis();
        FATDOG_LOG_INFO(g_logger) << "main begin";
        fatdog::Fiber::ptr fiber(new fatdog::Fiber(run_in_fiber, 1024 * 1024));
        FATDOG_LOG_INFO(g_logger) << "guagua";
        fiber->swapIn();
        FATDOG_LOG_INFO(g_logger) << "main after swapIn";
        fiber->swapIn();
        FATDOG_LOG_INFO(g_logger) << "main after end";
        fiber->swapIn();
    }
    FATDOG_LOG_INFO(g_logger) << "main after end2";
}

void thread_run()
{
    std::vector<fatdog::Thread::ptr> thrs;
    for(int i = 0; i < 1; ++i) {
        thrs.push_back(fatdog::Thread::ptr(
                    new fatdog::Thread("name_" + std::to_string(i), &test_fiber)));
    }
    for(auto i : thrs) {
        i->join();
    }
}




void test_backtrace()
{
    // FATDOG_LOG_INFO(FATDOG_LOG_ROOT()) << fatdog::BacktraceToString(10, 2, "lala");

    // FATDOG_ASSERT(1 + 1 > 2);
    // FATDOG_ASSERT2(1 + 1 > 2, "just try it");
}

#define MAX_COUNT 5

static ucontext_t uc[3];
static int count = 0;

void ping();
void pong();

void ping(){
    while(count < MAX_COUNT){
        FATDOG_LOG_INFO(FATDOG_LOG_ROOT()) << "in ping: " << ++count;
        // yield to pong
        swapcontext(&uc[1], &uc[2]); // 保存当前context于uc[1],切换至uc[2]的context运行
    }
}

void pong(){
    while(count < MAX_COUNT){
        FATDOG_LOG_INFO(FATDOG_LOG_ROOT()) << "in pong: " << ++count;
        // yield to ping
        swapcontext(&uc[2], &uc[1]);// 保存当前context于uc[2],切换至uc[1]的context运行
    }
}

void test_ucontext1()
{
    char st1[8192];
    char st2[8192];

    FATDOG_LOG_INFO(FATDOG_LOG_ROOT()) << "in test_ucontext begin ";
    // initialize context
    getcontext(&uc[1]);
    getcontext(&uc[2]);

    uc[1].uc_link = &uc[0]; // 这个玩意表示uc[1]运行完成后，会跳至uc[0]指向的context继续运行
    uc[1].uc_stack.ss_sp = st1; // 设置新的堆栈
    uc[1].uc_stack.ss_size = sizeof st1;
    FATDOG_LOG_INFO(FATDOG_LOG_ROOT()) << "before makecontext 1";
    makecontext (&uc[1], ping, 0);
    FATDOG_LOG_INFO(FATDOG_LOG_ROOT()) << "after makecontext 1";

    uc[2].uc_link = &uc[0]; // 这个玩意表示uc[2]运行完成后，会跳至uc[0]指向的context继续运行
    uc[2].uc_stack.ss_sp = st2; // 设置新的堆栈
    uc[2].uc_stack.ss_size = sizeof st2;
    FATDOG_LOG_INFO(FATDOG_LOG_ROOT()) << "before makecontext 2";
    makecontext (&uc[2], pong, 0);
    FATDOG_LOG_INFO(FATDOG_LOG_ROOT()) << "after makecontext 2";

    // start ping-pong
    swapcontext(&uc[0], &uc[1]); // 将当前context信息保存至uc[0],跳转至uc[1]保存的context去执行
    // 这里我稍微要多说几句，因为我迷惑过，我曾经困惑的一点在于uc[0]，为什么uc[0]不需要设置堆栈的信息？因为swapcontext已经帮我们做好了一切，swapcontext函数会将当前点的信息保存在uc[0]中，当然我们没有设置的话，默认的堆栈一定是主堆栈啦
    FATDOG_LOG_INFO(FATDOG_LOG_ROOT()) << "in test_ucontext end ";
}

/*
 * setcontext() 结束后会回到 getcontext() 后
 * swapcontext() 结束后会回到 swapcontext() 后
*/
void test_ucontext2()
{
    ucontext_t context;
    
    //获取当前程序上下文
    getcontext(&context);
    FATDOG_LOG_INFO(FATDOG_LOG_ROOT()) << "hello world";
    sleep(1);
    //将程序切换至context指向的上下文处
    setcontext(&context);
}

int main()
{
    // test_backtrace();
    // test_ucontext1();
     test_ucontext2();

    // thread_run();

    return 0;
}