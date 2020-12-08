#ifndef __FATDOG_THREAD_H__
#define __FATDOG_THREAD_H__

#include <memory>
#include <string>
#include <functional>

#include <pthread.h>
#include <semaphore.h>

namespace fatdog
{
    class Semaphore
    {
    private:
        Semaphore(const Semaphore &) = delete;
        Semaphore(const Semaphore &&) = delete;
        Semaphore &operator=(const Semaphore &) = delete;

    public:
        Semaphore(uint32_t count = 0);
        ~Semaphore();

        int wait();     // sub 1
        int notify();   // add 1

    private:
        sem_t m_sem;
    };

    template <class T>
    class ScopedLockImpl
    {
    public:
        ScopedLockImpl(T &mutex)
            : m_mutex(mutex)
        {
            m_mutex.lock();
            m_locked = true;
        }

        ~ScopedLockImpl()
        {
            unlock();
        }

        void lock()
        {
            if (!m_locked)
            {
                m_mutex.lock();
                m_locked = true;
            }
        }

        void unlock()
        {
            if (m_locked)
            {
                m_mutex.unlock();
                m_locked = false;
            }
        }

    private:
        T &m_mutex;
        bool m_locked;
    };

    template <class T>
    struct ReadScopedLockImpl
    {
    public:
        ReadScopedLockImpl(T &mutex)
            : m_mutex(mutex)
        {
            m_mutex.rdlock();
            m_locked = true;
        }

        ~ReadScopedLockImpl()
        {
            unlock();
        }

        void lock()
        {
            if (!m_locked)
            {
                m_mutex.rdlock();
                m_locked = true;
            }
        }

        void unlock()
        {
            if (m_locked)
            {
                m_mutex.unlock();
                m_locked = false;
            }
        }

    private:
        T &m_mutex;
        bool m_locked;
    };

    template <class T>
    struct WriteScopedLockImpl
    {
    public:
        WriteScopedLockImpl(T &mutex)
            : m_mutex(mutex)
        {
            m_mutex.wrlock();
            m_locked = true;
        }

        ~WriteScopedLockImpl()
        {
            unlock();
        }

        void lock()
        {
            if (!m_locked)
            {
                m_mutex.wrlock();
                m_locked = true;
            }
        }

        void unlock()
        {
            if (m_locked)
            {
                m_mutex.unlock();
                m_locked = false;
            }
        }

    private:
        T &m_mutex;
        bool m_locked;
    };

    class RWMutex
    {
    public:
        typedef ReadScopedLockImpl<RWMutex> ReadLock;
        typedef WriteScopedLockImpl<RWMutex> WriteLock;

        RWMutex()
        {
            pthread_rwlock_init(&m_lock, nullptr);
        }

        ~RWMutex()
        {
            pthread_rwlock_destroy(&m_lock);
        }

        void rdlock()
        {
            pthread_rwlock_rdlock(&m_lock);
        }

        void wrlock()
        {
            pthread_rwlock_wrlock(&m_lock);
        }

        void unlock()
        {
            pthread_rwlock_unlock(&m_lock);
        }

    private:
        pthread_rwlock_t m_lock;
    };

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

        Semaphore m_semaphore;
    };
} // namespace fatdog

#endif