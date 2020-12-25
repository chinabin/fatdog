#ifndef __FATDOG_HTTP_HTTP_SERVER_H__
#define __FATDOG_HTTP_HTTP_SERVER_H__

#include "../tcp_server.h"
#include "http_session.h"
#include "servlet.h"

namespace fatdog
{
    namespace http
    {

        class HttpServer : public TcpServer
        {
        public:
            typedef std::shared_ptr<HttpServer> ptr;
            HttpServer(bool keepalive = false, fatdog::IOManager *worker = fatdog::IOManager::GetThis(), fatdog::IOManager *accept_worker = fatdog::IOManager::GetThis());

            ServletDispatch::ptr getServletDispatch() const { return m_dispatch; }
            void setServletDispatch(ServletDispatch::ptr v) { m_dispatch = v; }

        protected:
            virtual void handleClient(Socket::ptr client) override;

        private:
            bool m_isKeepalive;
            ServletDispatch::ptr m_dispatch;
        };

    } // namespace http
} // namespace fatdog

#endif
