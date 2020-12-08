#include "thread.h"
#include "util.h"
#include "log.h"

static fatdog::Logger::ptr g_logger = FATDOG_LOG_NAME("system");

namespace fatdog
{
    static thread_local Thread *t_thread = nullptr;
    static thread_local std::string t_thread_name = "UNKNOW";

    Semaphore::Semaphore(uint32_t count)
    {
        int t = sem_init(&m_sem, 0, count);
        if (t)
        {
            FATDOG_LOG_ERROR(g_logger) << "sem_init failed, rt = " << t;
            throw std::logic_error("sem_init error");
        }
    }
    Semaphore::~Semaphore()
    {
        sem_destroy(&m_sem);
    }

    int Semaphore::wait()
    {
        if (sem_wait(&m_sem))
        {
            throw std::logic_error("sem_wait error");
        }
    }

    int Semaphore::notify()
    {
        if (sem_post(&m_sem))
        {
            throw std::logic_error("sem_post error");
        }
    }

    Thread::Thread(const std::string &name, std::function<void(void)> cb)
        : m_name(name), m_cb(cb)
    {
        int t = pthread_create(&m_thread, nullptr, run, this);
        if (t)
        {
            FATDOG_LOG_ERROR(g_logger) << "pthread_create failed, rt = " << t << ", name is " << name;
            throw std::logic_error("pthread_create error");
        }

        m_semaphore.wait();
    }

    Thread::~Thread()
    {
        if (m_thread)
            pthread_detach(m_thread);
    }

    Thread *Thread::GetThis()
    {
        return t_thread;
    }

    const std::string &Thread::GetName()
    {
        return t_thread_name;
    }

    void Thread::join()
    {
        if (m_thread)
        {
            int rt = pthread_join(m_thread, nullptr);
            if (rt)
            {
                FATDOG_LOG_ERROR(g_logger) << "pthread_join thread fail, rt=" << rt
                                           << " name=" << m_name;
                throw std::logic_error("pthread_join error");
            }
            m_thread = 0;
        }
    }

    void *Thread::run(void *arg)
    {
        Thread *p = (Thread *)arg;
        t_thread = p;
        t_thread_name = p->getName();
        p->m_id = fatdog::GetThreadId();

        // why pthread_self, but not m_thread
        pthread_setname_np(pthread_self(), p->m_name.substr(0, 15).c_str());

        std::function<void(void)> cb;
        cb.swap(p->m_cb);

        p->m_semaphore.notify();

        cb();

        return nullptr;
    }
} // namespace fatdog