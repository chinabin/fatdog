#ifndef __FATDOG_FIBER_H__
#define __FATDOG_FIBER_H__

#include <memory>
#include <functional>
#include <ucontext.h>

namespace fatdog
{
    class Fiber : public std::enable_shared_from_this<Fiber>
    {
    public:
        enum State
        {
            INIT,
            HOLD,
            EXEC,
            TERM,
            READY,
            EXCEPT
        };

        typedef std::shared_ptr<Fiber> ptr;
        Fiber(std::function<void(void)>, size_t stacksize = 1024 * 1024);
        ~Fiber();

    public:
        void reset(std::function<void(void)>);
        void swapIn();
        void swapOut();

        uint64_t getId() const { return m_id; }
        State getState() const { return m_state; }

    public:
        static void SetThis(Fiber *f);
        static Fiber::ptr GetThis();
        static void YieldToReady();
        static void YieldToHold();
        static uint64_t TotalFibers();

        static void MainFunc();
        static uint64_t GetFiberId();

    private:
        Fiber();

        uint64_t m_id = 0;
        uint32_t m_stacksize = 0;
        State m_state = INIT;

        ucontext_t m_ctx;
        void *m_stack = nullptr;

        std::function<void(void)> m_cb;
    };
} // namespace fatdog

#endif