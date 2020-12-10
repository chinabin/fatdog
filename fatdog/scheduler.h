#ifndef __FATDOG_SCHEDULER_H__
#define __FATDOG_SCHEDULER_H__

#include <memory>
#include <string>
#include <functional>
#include <vector>
#include <list>

#include "fiber.h"
#include "thread.h"

namespace fatdog
{
    class Scheduler
    {
    public:
        typedef std::shared_ptr<Scheduler> ptr;

        Scheduler(const std::string &name, uint32_t threads = 1, bool use_caller = true);
        virtual ~Scheduler();

        const std::string &getName() const { return m_name; }

        void start();
        void stop();

        static Fiber *GetMainFiber();
        static Scheduler *GetThis();

    public:
        template <class FiberOrCb>
        void schedule(FiberOrCb f, int thread = -1)
        {
            FiberAndThread ft(f, thread);
            if (ft.fiber || ft.cb)
            {
                m_fibers.push_back({f, thread});
            }
        }

    private:
        class FiberAndThread
        {
        public:
            FiberAndThread(Fiber::ptr f, int t = -1)
                : fiber(f), cb(nullptr), thread(t)
            {
            }

            FiberAndThread(std::function<void()> f, int t = -1)
                : fiber(nullptr), cb(f), thread(t)
            {
            }

            FiberAndThread()
                : thread(-1)
            {
            }

            void reset()
            {
                fiber = nullptr;
                cb = nullptr;
                thread = -1;
            }

            Fiber::ptr fiber;
            std::function<void()> cb;
            int thread;
        };

    protected:
        void run();
        void setThis();
        virtual void idle();

    protected:
        size_t m_threadCount = 0;

    private:
        std::string m_name;
        Fiber::ptr m_rootFiber;
        std::vector<Thread::ptr> m_threads;
        std::list<FiberAndThread> m_fibers;
    };
} // namespace fatdog

#endif