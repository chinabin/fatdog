#include "scheduler.h"
#include "log.h"
#include "macro.h"
#include "thread.h"

namespace fatdog
{
    static Logger::ptr g_logger = FATDOG_LOG_ROOT();

    static thread_local Scheduler *t_scheduler = nullptr;
    static thread_local Fiber *t_fiber = nullptr;

    Scheduler::Scheduler(const std::string &name, uint32_t threads, bool use_caller)
        : m_name(name)
    {
        FATDOG_LOG_INFO(g_logger) << "Scheduler()";
        if (use_caller)
        {
            Fiber::GetThis();
            --threads;

            t_scheduler = this;
            m_rootFiber.reset(new Fiber(std::bind(&Scheduler::run, this), 1024 * 1024, true));
            t_fiber = m_rootFiber.get();
        }
        else
        {
        }

        m_threadCount = threads;
    }

    Scheduler::~Scheduler()
    {
        if (GetThis() == this)
        {
            t_scheduler = nullptr;
        }
    }

    void Scheduler::start()
    {
        FATDOG_LOG_INFO(g_logger) << "Scheduler::start()";
        m_threads.resize(m_threadCount);
        for (size_t i = 0; i < m_threadCount; ++i)
        {
            m_threads[i].reset(new Thread(m_name + "_" + std::to_string(i), std::bind(&Scheduler::run, this)));
        }

        if (m_rootFiber)
        {
            //m_rootFiber->swapIn();
            m_rootFiber->call();
            FATDOG_LOG_INFO(g_logger) << "call out " << m_rootFiber->getState();
        }
    }

    void Scheduler::stop()
    {
        FATDOG_LOG_INFO(g_logger) << "Scheduler::stop()";
        if (m_rootFiber && m_threadCount == 0 && (m_rootFiber->getState() == Fiber::TERM || m_rootFiber->getState() == Fiber::INIT))
        {
            FATDOG_LOG_INFO(g_logger) << this << " stopped";
        }
    }

    Fiber *Scheduler::GetMainFiber()
    {
        return t_fiber;
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
        setThis();
        t_fiber = Fiber::GetThis().get();

        Fiber::ptr idle_fiber(new Fiber(std::bind(&Scheduler::idle, this)));
        Fiber::ptr cb_fiber;

        FiberAndThread ft;
        while (true)
        {
            ft.reset();
            {
                auto it = m_fibers.begin();
                while (it != m_fibers.end())
                {
                    if (it->thread != -1 && it->thread != fatdog::GetThreadId())
                    {
                        ++it;
                        continue;
                    }

                    if (it->fiber && it->fiber->getState() == Fiber::EXEC)
                    {
                        ++it;
                        continue;
                    }

                    ft = *it;
                    m_fibers.erase(it);
                    break;
                }
            }

            if (ft.fiber && (ft.fiber->getState() != Fiber::TERM && ft.fiber->getState() != Fiber::EXCEPT))
            {
                ft.fiber->swapIn();

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
                if (idle_fiber->getState() == Fiber::TERM)
                {
                    FATDOG_LOG_INFO(g_logger) << "idle fiber term";
                    break;
                }

                idle_fiber->swapIn();
                if (idle_fiber->getState() != Fiber::TERM && idle_fiber->getState() != Fiber::EXCEPT)
                {
                    idle_fiber->setState(Fiber::HOLD);
                }
            }
        }
        FATDOG_LOG_INFO(g_logger) << "Scheduler::run bye";
    }

    void Scheduler::idle()
    {
        FATDOG_LOG_INFO(g_logger) << "idle";
    }
} // namespace fatdog