#include "timer.h"
#include "util.h"

namespace fatdog
{
    bool TimerManager::Timer::Comparator::operator()(const Timer::ptr &lhs, const Timer::ptr &rhs) const
    {
        if (!lhs && !rhs)
        {
            return false;
        }
        if (!lhs)
        {
            return true;
        }
        if (!rhs)
        {
            return false;
        }
        if (lhs->m_next < rhs->m_next)
        {
            return true;
        }
        if (rhs->m_next < lhs->m_next)
        {
            return false;
        }
        return lhs.get() < rhs.get();
    }

    TimerManager::Timer::Timer(uint64_t ms, std::function<void()> cb,
                               bool recurring, TimerManager *manager)
        : m_recurring(recurring), m_ms(ms), m_cb(cb), m_manager(manager)
    {
        m_next = fatdog::GetCurrentMS() + m_ms;
    }

    TimerManager::Timer::Timer(uint64_t next)
        : m_next(next)
    {
    }

    bool TimerManager::Timer::cancel()
    {
        if (m_cb)
        {
            m_cb = nullptr;
            auto it = m_manager->m_timers.find(shared_from_this());
            m_manager->m_timers.erase(it);
            return true;
        }
        return false;
    }

    bool TimerManager::Timer::refresh()
    {
        if (!m_cb)
        {
            return false;
        }
        auto it = m_manager->m_timers.find(shared_from_this());
        if (it == m_manager->m_timers.end())
        {
            return false;
        }
        m_manager->m_timers.erase(it);
        m_next = fatdog::GetCurrentMS() + m_ms;
        m_manager->m_timers.insert(shared_from_this());
        return true;
    }

    bool TimerManager::Timer::reset(uint64_t ms, bool from_now)
    {
        if (ms == m_ms && !from_now)
        {
            return true;
        }

        if (!m_cb)
        {
            return false;
        }
        auto it = m_manager->m_timers.find(shared_from_this());
        if (it == m_manager->m_timers.end())
        {
            return false;
        }
        m_manager->m_timers.erase(it);
        uint64_t start = 0;
        if (from_now)
        {
            start = fatdog::GetCurrentMS();
        }
        else
        {
            start = m_next - m_ms; // 回到当年那个时间点作为开始
        }
        m_ms = ms;
        m_next = start + m_ms;
        m_manager->addTimer(shared_from_this());
        return true;
    }

    TimerManager::TimerManager()
    {
    }

    TimerManager::~TimerManager()
    {
    }

    TimerManager::Timer::ptr TimerManager::addTimer(uint64_t ms, std::function<void()> cb, bool recurring)
    {
        Timer::ptr timer(new Timer(ms, cb, recurring, this));
        addTimer(timer);
        return timer;
    }

    static void OnTimer(std::weak_ptr<void> weak_cond, std::function<void()> cb)
    {
        std::shared_ptr<void> tmp = weak_cond.lock();
        if (tmp)
        {
            cb();
        }
    }

    TimerManager::Timer::ptr TimerManager::addConditionTimer(uint64_t ms, std::function<void()> cb, std::weak_ptr<void> weak_cond, bool recurring)
    {
        return addTimer(ms, std::bind(&OnTimer, weak_cond, cb), recurring);
    }

    uint64_t TimerManager::getNextTimer()
    {
        m_tickled = false;
        if (m_timers.empty())
        {
            return ~0ull;
        }

        const TimerManager::Timer::ptr &next = *m_timers.begin();
        uint64_t now_ms = fatdog::GetCurrentMS();
        if (now_ms >= next->getNext())
        {
            return 0;
        }
        else
        {
            return next->getNext() - now_ms;
        }
    }

    void TimerManager::listExpiredCb(std::vector<std::function<void()>> &cbs)
    {
        uint64_t now_ms = fatdog::GetCurrentMS();
        std::vector<Timer::ptr> expired;
        {
            if (m_timers.empty())
            {
                return;
            }
        }

        if (m_timers.empty())
        {
            return;
        }

        Timer::ptr now_timer(new Timer(now_ms));
        auto it = m_timers.lower_bound(now_timer); // https://en.cppreference.com/w/cpp/container/set/lower_bound
        while (it != m_timers.end() && (*it)->getNext() == now_ms)
        {
            ++it;
        }
        expired.insert(expired.begin(), m_timers.begin(), it);
        m_timers.erase(m_timers.begin(), it);
        cbs.reserve(expired.size());

        for (auto &timer : expired)
        {
            cbs.push_back(timer->getF());
            if (timer->getRecurring())
            {
                timer->getNext() = now_ms + timer->getMs();
                m_timers.insert(timer);
            }
            else
            {
                timer->getF() = nullptr;
            }
        }
    }

    void TimerManager::addTimer(TimerManager::Timer::ptr val)
    {
        auto it = m_timers.insert(val).first;
        bool at_front = (it == m_timers.begin()) && !m_tickled;
        if (at_front)
        {
            m_tickled = true;
        }

        if (at_front)
        {
            onTimerInsertedAtFront();
        }
    }

    bool TimerManager::hasTimer()
    {
        return !m_timers.empty();
    }
} // namespace fatdog