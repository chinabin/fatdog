#include "fiber.h"

#include "log.h"
#include "macro.h"

#include <atomic>

namespace fatdog
{
    static Logger::ptr g_logger = FATDOG_LOG_ROOT();

    static std::atomic<uint64_t> s_fiber_id{0};
    static std::atomic<uint64_t> s_fiber_count{0};

    static thread_local Fiber *t_fiber;                     // 当前协程
    static thread_local Fiber::ptr t_threadFiber = nullptr; // 线程主协程

    class StackMalloc
    {
    public:
        static void *Alloc(size_t s)
        {
            return malloc(s);
        }

        static void Dealloc(void *p)
        {
            return free(p);
        }
    };

    using StackAllocator = StackMalloc;

    Fiber::Fiber()
    {
        m_state = EXEC;
        SetThis(this);

        if (getcontext(&m_ctx))
        {
            FATDOG_ASSERT2(false, "getcontext");
        }
        ++s_fiber_count;

        FATDOG_LOG_INFO(g_logger) << "Fiber::Fiber";
    }

    Fiber::Fiber(std::function<void(void)> cb, size_t stacksize)
        : m_id(++s_fiber_id), m_cb(cb)
    {
        m_state = INIT;
        ++s_fiber_count;
        m_stacksize = stacksize;
        m_stack = StackAllocator::Alloc(m_stacksize);

        if (getcontext(&m_ctx))
        {
            FATDOG_ASSERT2(false, "getcontext");
        }

        m_ctx.uc_link = nullptr;
        m_ctx.uc_stack.ss_size = m_stacksize;
        m_ctx.uc_stack.ss_sp = m_stack;

        makecontext(&m_ctx, Fiber::MainFunc, 0);

        FATDOG_LOG_INFO(g_logger) << "Fiber::Fiber id=" << m_id;
    }

    Fiber::Fiber::~Fiber()
    {
        --s_fiber_count;
        if (m_stack)
        {
            FATDOG_ASSERT(m_state == INIT || m_state == TERM || m_state == EXCEPT);
            StackAllocator::Dealloc(m_stack);
        }
        else
        {
            FATDOG_ASSERT(!m_cb);
            FATDOG_ASSERT(m_state == EXEC);

            Fiber *cur = t_fiber;
            if (cur == this)
            {
                SetThis(nullptr);
            }
        }

        FATDOG_LOG_INFO(g_logger) << "Fiber::~Fiber id=" << m_id;
    }

    uint64_t Fiber::GetFiberId()
    {
        if (t_fiber)
        {
            return t_fiber->getId();
        }
        return 0;
    }

    void Fiber::reset(std::function<void(void)> cb)
    {
        FATDOG_ASSERT(m_state == INIT || m_state == TERM || m_state == EXCEPT);
        FATDOG_ASSERT(m_stack);

        m_cb = cb;
        if (getcontext(&m_ctx))
        {
            FATDOG_ASSERT2(false, "getcontext");
        }

        m_ctx.uc_link = nullptr;
        m_ctx.uc_stack.ss_size = m_stacksize;
        m_ctx.uc_stack.ss_sp = m_stack;

        makecontext(&m_ctx, Fiber::MainFunc, 0);
        m_state = INIT;
    }

    void Fiber::swapIn()
    {
        FATDOG_LOG_DEBUG(g_logger) << "lalalalaalal";
        FATDOG_LOG_DEBUG(g_logger) << "from " << GetThis()->getId() << " swap in to " << getId();
        SetThis(this);
        FATDOG_ASSERT(m_state != EXEC);
        m_state = EXEC;
        if (swapcontext(&(t_threadFiber->m_ctx), &m_ctx))
        {
            FATDOG_ASSERT2(false, "swapcontext");
        }
    }

    void Fiber::swapOut()
    {
        FATDOG_LOG_DEBUG(g_logger) << "from " << GetThis()->getId() << " swap out to " << t_threadFiber->getId();
        SetThis(t_threadFiber.get());

        if (swapcontext(&m_ctx, &(t_threadFiber->m_ctx)))
        {
            FATDOG_ASSERT2(false, "swapcontext");
        }
    }

    void Fiber::SetThis(Fiber *f)
    {
        t_fiber = f;
    }

    Fiber::ptr Fiber::GetThis()
    {
        if (t_fiber)
        {
            return t_fiber->shared_from_this();
        }
        Fiber::ptr main_fiber(new Fiber);
        FATDOG_ASSERT(t_fiber == main_fiber.get());
        t_threadFiber = main_fiber;
        return t_fiber->shared_from_this();
    }

    void Fiber::YieldToReady()
    {
        Fiber::ptr cur = GetThis();
        cur->m_state = READY;
        cur->swapOut();
    }

    void Fiber::YieldToHold()
    {
        Fiber::ptr cur = GetThis();
        cur->m_state = HOLD;
        cur->swapOut();
    }

    uint64_t Fiber::TotalFibers()
    {
        return s_fiber_count;
    }

    void Fiber::MainFunc()
    {
        Fiber::ptr cur = GetThis();
        FATDOG_ASSERT(cur);
        try
        {
            cur->m_cb();
            cur->m_cb = nullptr;
            cur->m_state = TERM;
        }
        catch (std::exception &ex)
        {
            cur->m_state = EXCEPT;
            FATDOG_LOG_ERROR(g_logger) << "Fiber Except: " << ex.what()
                                       << " fiber_id=" << cur->getId()
                                       << std::endl
                                       << fatdog::BacktraceToString(64);
        }
        catch (...)
        {
            cur->m_state = EXCEPT;
            FATDOG_LOG_ERROR(g_logger) << "Fiber Except"
                                       << " fiber_id=" << cur->getId()
                                       << std::endl
                                       << fatdog::BacktraceToString(64);
        }

        auto raw_ptr = cur.get();
        cur.reset();
        raw_ptr->swapOut();

        FATDOG_ASSERT2(false, "never reach fiber_id=" + std::to_string(raw_ptr->getId()));
    }
} // namespace fatdog