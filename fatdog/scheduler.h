#ifndef __FATDOG_SCHEDULER_H__
#define __FATDOG_SCHEDULER_H__

#include <memory>
#include <string>
#include <functional>
#include <vector>
#include <list>

#include "fiber.h"
#include "thread.h"

/*
 * scheduler's purpose is to coordinate fibers.
 * scene1, no inject(i.e. use_caller = false)
 *      1. call start() to produce m_threadCount threads, each 
 *          thread call run(), which bind by this pointer, and
 *          each thread will start by itself.
 *          in run(), produce one main fiber and one idle fiber.
 *          so, if have something to do, do it, if not, go in idle.
 *          (you should decide when from run to idle, when back and how)
 *      2. call scheduler(f) to run. (you should promise f must run successfully.)
 *      3. call stop to end. just call thread.join() to wait thread end, it means
 *          run() end, which means idle fiber end and no work to do. (main fiber and
 *          idle fiber must clean up)
 * 
 * scene2, with inject only(i.e. use_caller = true, threads = 1)
 *      1. at scheduler's constructor, produce one main fiber, which will call run().
 *          (you should decide when and how to call run())
 *      2. same
 *      3. make scheduler stop, and let control back to original thread, keep going where 
 *          it should be.
 * 
 * scene3, with inject and spawn threads(i.e. use_caller = true, threads > 1)
*/

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

        static Fiber *GetMainFiber(); // return current scheduler's main fiber
        static Scheduler *GetThis();  // return current scheduler

    public:
        template <class FiberOrCb>
        void schedule(FiberOrCb f, int thread = -1)
        {
            bool need_tickle = m_fibers.empty();

            FiberAndThread ft(f, thread);
            if (ft.fiber || ft.cb)
            {
                m_fibers.push_back({f, thread});
            }

            if (need_tickle)
            {
                tickle();
            }
        }

        template <class InputIterator>
        void schedule(InputIterator begin, InputIterator end)
        {
            while (begin != end)
            {
                schedule(*begin, -1);
                ++begin;
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
        virtual void tickle();
        void run();
        void setThis();
        virtual void idle();
        virtual bool stopping(); // indicate if can stop

        bool hasIdleThreads() { return m_idleThreadCount > 0; }

    protected:
        size_t m_threadCount = 0;
        int m_rootThread = 0; // use_caller's thread id
        bool m_stopping = true;
        std::atomic<size_t> m_activeThreadCount = {0};
        std::atomic<size_t> m_idleThreadCount = {0};

    private:
        std::string m_name;
        Fiber::ptr m_rootFiber;
        std::vector<Thread::ptr> m_threads;
        std::list<FiberAndThread> m_fibers;
    };
} // namespace fatdog

#endif