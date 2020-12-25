#include "tcp_server.h"
#include "config.h"
#include "log.h"

namespace fatdog
{

    static fatdog::Logger::ptr g_logger = FATDOG_LOG_ROOT();

    TcpServer::TcpServer(fatdog::IOManager *woker,
                         fatdog::IOManager *accept_woker)
        : m_worker(woker), m_acceptWorker(accept_woker), m_recvTimeout(60 * 1000 * 2), m_name("fatdog/1.0.0"), m_isStop(true)
    {
    }

    TcpServer::~TcpServer()
    {
        for (auto &i : m_socks)
        {
            i->close();
        }
        m_socks.clear();
    }

    bool TcpServer::bind(fatdog::Address::ptr addr)
    {
        std::vector<Address::ptr> addrs;
        std::vector<Address::ptr> fails;
        addrs.push_back(addr);
        return bind(addrs, fails);
    }

    bool TcpServer::bind(const std::vector<Address::ptr> &addrs, std::vector<Address::ptr> &fails)
    {
        for (auto &addr : addrs)
        {
            Socket::ptr sock = Socket::CreateTCP(addr);
            if (!sock->bind(addr))
            {
                FATDOG_LOG_ERROR(g_logger) << "bind fail errno="
                                           << errno << " errstr=" << strerror(errno)
                                           << " addr=[" << addr->toString() << "]";
                fails.push_back(addr);
                continue;
            }
            if (!sock->listen())
            {
                FATDOG_LOG_ERROR(g_logger) << "listen fail errno="
                                           << errno << " errstr=" << strerror(errno)
                                           << " addr=[" << addr->toString() << "]";
                fails.push_back(addr);
                continue;
            }
            m_socks.push_back(sock);
        }

        if (!fails.empty())
        {
            m_socks.clear();
            return false;
        }

        for (auto &i : m_socks)
        {
            FATDOG_LOG_INFO(g_logger) << "server bind success: " << *i;
        }
        return true;
    }

    void TcpServer::startAccept(Socket::ptr sock)
    {
        while (!m_isStop)
        {
            Socket::ptr client = sock->accept();
            if (client)
            {
                client->setRecvTimeout(m_recvTimeout);
                m_worker->schedule(std::bind(&TcpServer::handleClient,
                                             shared_from_this(), client));
            }
            else
            {
                FATDOG_LOG_ERROR(g_logger) << "accept errno=" << errno
                                           << " errstr=" << strerror(errno);
            }
        }
    }

    bool TcpServer::start()
    {
        if (!m_isStop)
        {
            return true;
        }
        m_isStop = false;
        for (auto &sock : m_socks)
        {
            m_acceptWorker->schedule(std::bind(&TcpServer::startAccept,
                                               shared_from_this(), sock));
        }
        return true;
    }

    void TcpServer::stop()
    {
        m_isStop = true;
        auto self = shared_from_this();
        m_acceptWorker->schedule([this, self]() {
            for (auto &sock : m_socks)
            {
                sock->cancelAll();
                sock->close();
            }
            m_socks.clear();
        });
    }

    void TcpServer::handleClient(Socket::ptr client)
    {
        FATDOG_LOG_INFO(g_logger) << "handleClient: " << *client;
    }

} // namespace fatdog
