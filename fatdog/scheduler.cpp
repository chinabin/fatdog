#include "scheduler.h"
#include "log.h"
#include "macro.h"
#include "thread.h"
#include "hook.h"

namespace fatdog
{
    static Logger::ptr g_logger = FATDOG_LOG_ROOT();

    static thread_local Scheduler *t_scheduler = nullptr;
    static thread_local Fiber *t_main_fiber = nullptr; // current scheduler's main fiber

    Scheduler::Scheduler(const std::string &name, uint32_t threads, bool use_caller)
        : m_name(name)
    {
        FATDOG_ASSERT(threads > 0);
        if (use_caller)
        {
            fatdog::Thread::SetName(m_name);
            Fiber::GetThis();
            --threads;

            t_scheduler = this;
            m_rootFiber.reset(new Fiber(std::bind(&Scheduler::run, this), 1024 * 1024, true));
            t_main_fiber = m_rootFiber.get();
            m_rootThread = fatdog::GetThreadId();
        }
        else
        {
            m_rootThread = -1;
        }

        m_threadCount = threads;
    }

    Scheduler::~Scheduler()
    {
        if (GetThis() == this)
        {
            t_scheduler = nullptr;
        }
        FATDOG_LOG_INFO(g_logger) << "Scheduler::~Scheduler()";
    }

    void Scheduler::start()
    {
        if (!m_stopping) // in case multiple start
        {
            return;
        }
        m_stopping = false;

        FATDOG_LOG_INFO(g_logger) << "Scheduler::start()";

        m_threads.resize(m_threadCount);
        for (size_t i = 0; i < m_threadCount; ++i)
        {
            m_threads[i].reset(new Thread(m_name + "_" + std::to_string(i), std::bind(&Scheduler::run, this)));
        }

        // do not call m_rootFiber->call() or m_rootFiber->swapIn() here, see stop()
    }

    void Scheduler::stop()
    {
        FATDOG_LOG_INFO(g_logger) << "Scheduler::stop()";
        m_stopping = true;

        // use_call = true, threads = 1.
        // already stopped or didn't start yet
        if (m_rootFiber && m_threadCount == 0 && (m_rootFiber->getState() == Fiber::TERM || m_rootFiber->getState() == Fiber::INIT))
        {
            FATDOG_LOG_INFO(g_logger) << this << " stopped";
            if (stopping()) // why?
            {
                return;
            }
        }

        if (m_rootThread != -1)
        {
            // A thread-hijacking scheduler must be stopped
            // from within itself to return control to the
            // original thread
            FATDOG_ASSERT(GetThis() == this);
        }
        else
        {
            // A spawned-threads only scheduler cannot be stopped from within
            // itself... who would get control?
            FATDOG_ASSERT(GetThis() != this);
        }

        for (size_t i = 0; i < m_threadCount; ++i)
        {
            tickle();
        }

        if (m_rootFiber)
        {
            tickle();
        }

        // can't put this code in start(), because if you do that, once you call start(), you can't
        // call scheduler() to add function or fiber until start() return(because m_rootFiber->call() will go
        // into run() function), and after start() return which means run() end. now you add anything is meaningless.
        if (m_rootFiber)
        {
            if (!stopping()) // why?
            {
                m_rootFiber->call();
            }
        }

        std::vector<Thread::ptr> thrs;
        {
            thrs.swap(m_threads);
        }

        for (auto &i : thrs)
        {
            i->join();
        }
    }

    Fiber *Scheduler::GetMainFiber()
    {
        return t_main_fiber;
    }

    Scheduler *Scheduler::GetThis()
    {
        return t_scheduler;
    }

    void Scheduler::setThis()
    {
        t_scheduler = this;
    }

    void Scheduler::run()
    {
        FATDOG_LOG_INFO(g_logger) << "Scheduler::run";
        set_hook_enable(true);
        setThis();
        if (fatdog::GetThreadId() != m_rootThread)
        {
            // because each thread will call Scheduler::run(), they mush have their own main fiber
            t_main_fiber = Fiber::GetThis().get();
        }

        Fiber::ptr idle_fiber(new Fiber(std::bind(&Scheduler::idle, this)));
        Fiber::ptr cb_fiber;

        FiberAndThread ft;
        while (true)
        {
            ft.reset();
            bool is_active = false;
            bool tickle_me = false;
            {
                auto it = m_fibers.begin();
                while (it != m_fibers.end())
                {
                    if (it->thread != -1 && it->thread != fatdog::GetThreadId())
                    {
                        ++it;
                        tickle_me = true;
                        continue;
                    }

                    if (it->fiber && it->fiber->getState() == Fiber::EXEC)
                    {
                        ++it;
                        continue;
                    }

                    ft = *it;
                    m_fibers.erase(it);
                    ++m_activeThreadCount;
                    is_active = true;
                    break;
                }
            }

            if (tickle_me)
            {
                tickle();
            }

            if (ft.fiber && ft.fiber->getState() != Fiber::TERM && ft.fiber->getState() != Fiber::EXCEPT)
            {
                ft.fiber->swapIn();
                --m_activeThreadCount;

                if (ft.fiber->getState() == Fiber::READY)
                {
                    schedule(ft.fiber);
                }
                else if (ft.fiber->getState() != Fiber::TERM && ft.fiber->getState() != Fiber::EXCEPT)
                {
                    ft.fiber->setState(Fiber::HOLD);
                }
                ft.reset();
            }
            else if (ft.cb)
            {
                if (cb_fiber)
                {
                    cb_fiber->reset(ft.cb);
                }
                else
                {
                    cb_fiber.reset(new Fiber(ft.cb));
                }
                ft.reset();
                cb_fiber->swapIn();
                --m_activeThreadCount;
                if (cb_fiber->getState() == Fiber::READY)
                {
                    schedule(cb_fiber);
                    cb_fiber.reset();
                }
                else if (cb_fiber->getState() == Fiber::EXCEPT || cb_fiber->getState() == Fiber::TERM)
                {
                    cb_fiber->reset(nullptr);
                }
                else
                { //if(cb_fiber->getState() != Fiber::TERM) {
                    cb_fiber->setState(Fiber::HOLD);
                    cb_fiber.reset();
                }
            }
            else
            {
                if (is_active)
                {
                    --m_activeThreadCount;
                    continue;
                }
                if (idle_fiber->getState() == Fiber::TERM)
                {
                    FATDOG_LOG_INFO(g_logger) << "idle fiber term";
                    break;
                }

                ++m_idleThreadCount;
                idle_fiber->swapIn();
                --m_idleThreadCount;
                if (idle_fiber->getState() != Fiber::TERM && idle_fiber->getState() != Fiber::EXCEPT)
                {
                    idle_fiber->setState(Fiber::HOLD);
                }
            }
        }
        FATDOG_LOG_INFO(g_logger) << "Scheduler::run bye";
    }

    bool Scheduler::stopping()
    {
        return m_stopping && m_fibers.empty() && m_activeThreadCount == 0;
    }

    void Scheduler::tickle()
    {
        FATDOG_LOG_INFO(g_logger) << "tickle";
    }

    void Scheduler::idle()
    {
        FATDOG_LOG_INFO(g_logger) << "idle";
        while (!stopping())
        {
            fatdog::Fiber::YieldToHold();
        }
    }
} // namespace fatdog