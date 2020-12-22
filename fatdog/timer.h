#ifndef __FATDOG_TIMER_H__
#define __FATDOG_TIMER_H__

#include <memory>
#include <set>
#include <vector>
#include <functional>

namespace fatdog
{
    class TimerManager
    {
    public:
        class Timer : public std::enable_shared_from_this<Timer>
        {
        public:
            typedef std::shared_ptr<Timer> ptr;
            Timer(uint64_t ms, std::function<void()> cb, bool recurring, TimerManager *manager);
            Timer(uint64_t next);

            bool cancel();
            bool refresh();
            bool reset(uint64_t ms, bool from_now);

            uint64_t& getNext() { return m_next; }
            uint64_t& getMs() { return m_ms; }
            bool getRecurring() const { return m_recurring; }
            std::function<void()>& getF() { return m_cb; }

            // 仿函数，为了 set
            struct Comparator
            {
                bool operator()(const Timer::ptr &lhs, const Timer::ptr &rhs) const;
            };

        private:
            bool m_recurring = false;
            uint64_t m_ms = 0;   // 执行周期
            uint64_t m_next = 0; // 精确时间
            std::function<void()> m_cb;
            TimerManager *m_manager = nullptr;
        };

    public:
        TimerManager();
        virtual ~TimerManager();

        Timer::ptr addTimer(uint64_t ms, std::function<void()> cb, bool recurring = false);
        uint64_t getNextTimer();
        void listExpiredCb(std::vector<std::function<void()>> &cbs);
        bool hasTimer();

    protected:
        virtual void onTimerInsertedAtFront() = 0;
        void addTimer(Timer::ptr val);

    private:
        std::set<Timer::ptr, Timer::Comparator> m_timers;
        /// 是否触发onTimerInsertedAtFront
        bool m_tickled = false;
    };
} // namespace fatdog

#endif