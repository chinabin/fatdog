#ifndef __FATDOG_HTTP_SERVLET_H__
#define __FATDOG_HTTP_SERVLET_H__

#include <memory>
#include <functional>
#include <string>
#include <vector>
#include <unordered_map>
#include "http.h"
#include "http_session.h"
#include "../thread.h"

namespace fatdog
{
    namespace http
    {

        class Servlet
        {
        public:
            typedef std::shared_ptr<Servlet> ptr;

            Servlet(const std::string &name)
                : m_name(name) {}
            virtual ~Servlet() {}
            virtual int32_t handle(fatdog::http::HttpRequest::ptr request, fatdog::http::HttpResponse::ptr response, fatdog::http::HttpSession::ptr session) = 0;

            const std::string &getName() const { return m_name; }

        protected:
            std::string m_name;
        };

        class FunctionServlet : public Servlet
        {
        public:
            typedef std::shared_ptr<FunctionServlet> ptr;
            typedef std::function<int32_t(fatdog::http::HttpRequest::ptr request, fatdog::http::HttpResponse::ptr response, fatdog::http::HttpSession::ptr session)> callback;

            FunctionServlet(callback cb);
            virtual int32_t handle(fatdog::http::HttpRequest::ptr request, fatdog::http::HttpResponse::ptr response, fatdog::http::HttpSession::ptr session) override;

        private:
            callback m_cb;
        };

        class ServletDispatch : public Servlet
        {
        public:
            typedef std::shared_ptr<ServletDispatch> ptr;
            typedef RWMutex RWMutexType;

            ServletDispatch();
            virtual int32_t handle(fatdog::http::HttpRequest::ptr request, fatdog::http::HttpResponse::ptr response, fatdog::http::HttpSession::ptr session) override;

            void addServlet(const std::string &uri, Servlet::ptr slt);
            void addServlet(const std::string &uri, FunctionServlet::callback cb);
            void addGlobServlet(const std::string &uri, Servlet::ptr slt);
            void addGlobServlet(const std::string &uri, FunctionServlet::callback cb);

            void delServlet(const std::string &uri);
            void delGlobServlet(const std::string &uri);

            Servlet::ptr getDefault() const { return m_default; }
            void setDefault(Servlet::ptr v) { m_default = v; }

            Servlet::ptr getServlet(const std::string &uri);
            Servlet::ptr getGlobServlet(const std::string &uri);

            Servlet::ptr getMatchedServlet(const std::string &uri);

        private:
            RWMutexType m_mutex;
            //uri(/fatdog/xxx) -> servlet
            std::unordered_map<std::string, Servlet::ptr> m_datas;
            //uri(/fatdog/*) -> servlet
            std::vector<std::pair<std::string, Servlet::ptr>> m_globs;
            //默认servlet，所有路径都没匹配到时使用
            Servlet::ptr m_default;
        };

        class NotFoundServlet : public Servlet
        {
        public:
            typedef std::shared_ptr<NotFoundServlet> ptr;
            NotFoundServlet();
            virtual int32_t handle(fatdog::http::HttpRequest::ptr request, fatdog::http::HttpResponse::ptr response, fatdog::http::HttpSession::ptr session) override;
        };

    } // namespace http
} // namespace fatdog

#endif
