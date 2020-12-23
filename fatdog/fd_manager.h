#ifndef __FATDOG_FD_MANAGER_H__
#define __FATDOG_FD_MANAGER_H__

#include <memory>
#include <vector>
#include "thread.h"
#include "singleton.h"

namespace fatdog
{

    class FdCtx : public std::enable_shared_from_this<FdCtx>
    {
    public:
        typedef std::shared_ptr<FdCtx> ptr;
        FdCtx(int fd);
        ~FdCtx();

        bool init();
        bool isInit() const { return m_isInit; }
        bool isSocket() const { return m_isSocket; }
        bool isClose() const { return m_isClosed; }
        bool close();

        void setUserNonblock(bool v) { m_userNonblock = v; }
        bool getUserNonblock() const { return m_userNonblock; }

        void setSysNonblock(bool v) { m_sysNonblock = v; }
        bool getSysNonblock() const { return m_sysNonblock; }

        void setTimeout(int type, uint64_t v);
        uint64_t getTimeout(int type);

    private:
        bool m_isInit = false;
        bool m_isSocket = false;
        bool m_sysNonblock = false;
        bool m_userNonblock = false;
        bool m_isClosed = false;
        int m_fd;
        uint64_t m_recvTimeout;
        uint64_t m_sendTimeout;
    };

    class FdManager
    {
    public:
        typedef RWMutex RWMutexType;
        FdManager();

        FdCtx::ptr get(int fd, bool auto_create = false);
        void del(int fd);

    private:
        std::vector<FdCtx::ptr> m_datas;
    };

    typedef Singleton<FdManager> FdMgr;

} // namespace fatdog

#endif
