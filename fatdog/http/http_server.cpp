#include "http_server.h"
#include "../log.h"

namespace fatdog
{
    namespace http
    {

        static fatdog::Logger::ptr g_logger = FATDOG_LOG_ROOT();

        HttpServer::HttpServer(bool keepalive, fatdog::IOManager *worker, fatdog::IOManager *accept_worker)
            : TcpServer(worker, accept_worker), m_isKeepalive(keepalive)
        {
            m_dispatch.reset(new ServletDispatch);
        }

        void HttpServer::handleClient(Socket::ptr client)
        {
            FATDOG_LOG_DEBUG(g_logger) << "handleClient " << *client;
            HttpSession::ptr session(new HttpSession(client));
            do
            {
                auto req = session->recvRequest();
                if (!req)
                {
                    FATDOG_LOG_DEBUG(g_logger) << "recv http request fail, errno="
                                               << errno << " errstr=" << strerror(errno)
                                               << " cliet:" << *client << " keep_alive=" << m_isKeepalive;
                    break;
                }

                HttpResponse::ptr rsp(new HttpResponse(req->getVersion(), req->isClose() || !m_isKeepalive));
                rsp->setHeader("Server", getName());
                m_dispatch->handle(req, rsp, session);
                session->sendResponse(rsp);
                if (!m_isKeepalive || req->isClose())
                {
                    break;
                }
            } while (true);
            session->close();
        }

    } // namespace http
} // namespace fatdog
