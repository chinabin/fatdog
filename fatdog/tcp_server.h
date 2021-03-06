#ifndef __FATDOG_TCP_SERVER_H__
#define __FATDOG_TCP_SERVER_H__

#include <memory>
#include <functional>
#include "address.h"
#include "iomanager.h"
#include "socket.h"
#include "noncopyable.h"

namespace fatdog
{

    class TcpServer : public std::enable_shared_from_this<TcpServer>, Noncopyable
    {
    public:
        typedef std::shared_ptr<TcpServer> ptr;
        TcpServer(fatdog::IOManager *woker = fatdog::IOManager::GetThis(), fatdog::IOManager *accept_woker = fatdog::IOManager::GetThis());
        virtual ~TcpServer();

        virtual bool bind(fatdog::Address::ptr addr);
        virtual bool bind(const std::vector<Address::ptr> &addrs, std::vector<Address::ptr> &fails);
        virtual bool start();
        virtual void stop();

        uint64_t getRecvTimeout() const { return m_recvTimeout; }
        std::string getName() const { return m_name; }
        void setRecvTimeout(uint64_t v) { m_recvTimeout = v; }
        void setName(const std::string &v) { m_name = v; }

        bool isStop() const { return m_isStop; }

    protected:
        virtual void handleClient(Socket::ptr client);
        virtual void startAccept(Socket::ptr sock);

    private:
        std::vector<Socket::ptr> m_socks;
        IOManager *m_worker;
        IOManager *m_acceptWorker;
        uint64_t m_recvTimeout;
        std::string m_name;
        bool m_isStop;
    };

} // namespace fatdog

#endif
