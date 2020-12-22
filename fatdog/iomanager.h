#ifndef __FATDOG_IOMANAGER_H__
#define __FATDOG_IOMANAGER_H__

#include "scheduler.h"
#include "timer.h"

namespace fatdog
{
    class IOManager : public Scheduler, public TimerManager
    {
    public:
        typedef std::shared_ptr<IOManager> ptr;

        enum Event
        {
            NONE = 0x0,
            READ = 0x1, // EPOLLIN
            WRITE = 0x4 // EPOLLOUT
        };

        IOManager(const std::string &name, size_t thread = 1, bool use_caller = true);
        ~IOManager();

        int addEvent(int fd, Event event, std::function<void()> cb = nullptr);
        bool delEvent(int fd, Event event);
        bool cancelEvent(int fd, Event event);

        bool cancelAll(int fd);

        static IOManager *GetThis();
        void contextResize(size_t size);

    protected:
        void tickle() override;
        bool stopping() override;
        void idle() override;
        void onTimerInsertedAtFront() override;
        bool stopping(uint64_t &timeout);

    private:
        /*
             * for the reason epoll_event.data's type is epoll_data_t, 
             * which is union, if we wanna carry more data, we must define
             * our data type, then point by epoll_event.data.ptr, that's why
             * we define FdContext .
            */
        struct FdContext
        {
            struct EventContext
            {
                Scheduler *scheduler = nullptr;
                Fiber::ptr fiber;
                std::function<void()> cb;
            };

            EventContext &getContext(Event event);
            void resetContext(EventContext &ctx);
            void triggerEvent(Event event);

            EventContext read;
            EventContext write;
            int fd = 0;
            Event events = NONE;
        };

    private:
        int m_epfd = 0;
        int m_tickleFds[2]; // 管道，0 读 1 写
        std::atomic<size_t> m_pendingEventCount = {0};
        std::vector<FdContext *> m_fdContexts;
    };
} // namespace fatdog

#endif