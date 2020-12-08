#ifndef __FATDOG_THREAD_H__
#define __FATDOG_THREAD_H__

#include <memory>
#include <string>
#include <functional>

#include <pthread.h>

namespace fatdog
{
    class Thread
    {
    private:
        Thread(const Thread &) = delete;
        Thread(const Thread &&) = delete;
        Thread &operator=(const Thread &) = delete;

        static void *run(void *arg); // for the reson of pthread_create, it must be static

    public:
        typedef std::shared_ptr<Thread> ptr;
        Thread(const std::string &name, std::function<void(void)> cb);
        ~Thread();

        pid_t getID() const { return m_id; }
        const std::string &getName() const { return m_name; }

        void join();

        static Thread *GetThis();
        static const std::string &GetName();

    private:
        std::string m_name;
        std::function<void(void)> m_cb;
        pthread_t m_thread = 0;
        pid_t m_id = -1; // https://blog.csdn.net/u013246898/article/details/52933275
    };
} // namespace fatdog

#endif