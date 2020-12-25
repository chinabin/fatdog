#include "../fatdog/http/http_server.h"
#include "../fatdog/log.h"

static fatdog::Logger::ptr g_logger = FATDOG_LOG_ROOT();

#define XX(...) #__VA_ARGS__

fatdog::IOManager::ptr woker;

void run()
{
    g_logger->setLevel(fatdog::LogLevel::INFO);
    //sylar::http::HttpServer::ptr server(new sylar::http::HttpServer(true, woker.get(), sylar::IOManager::GetThis()));
    fatdog::http::HttpServer::ptr server(new fatdog::http::HttpServer(true));
    fatdog::Address::ptr addr = fatdog::Address::LookupAnyIPAddress("0.0.0.0:8020");
    while (!server->bind(addr))
    {
        sleep(2);
    }
    auto sd = server->getServletDispatch();
    sd->addServlet("/fatdog/xx", [](fatdog::http::HttpRequest::ptr req, fatdog::http::HttpResponse::ptr rsp, fatdog::http::HttpSession::ptr session) {
        rsp->setBody(req->toString());
        return 0;
    });

    sd->addGlobServlet("/fatdog/*", [](fatdog::http::HttpRequest::ptr req, fatdog::http::HttpResponse::ptr rsp, fatdog::http::HttpSession::ptr session) {
        rsp->setBody("Glob:\r\n" + req->toString());
        return 0;
    });

    sd->addGlobServlet("/fatdogx/*", [](fatdog::http::HttpRequest::ptr req, fatdog::http::HttpResponse::ptr rsp, fatdog::http::HttpSession::ptr session) {
        rsp->setBody(XX(<html>
                                <head><title> 404 Not Found</ title></ head>
                                <body>
                                <center><h1> 404 Not Found</ h1></ center>
                                <hr><center>
                                    nginx /
                                1.16.0 <
                            / center >
                            </ body>
                            </ html> < !--a padding to disable MSIE and
                        Chrome friendly error page-- >
                            < !--a padding to disable MSIE and
                        Chrome friendly error page-- >
                            < !--a padding to disable MSIE and
                        Chrome friendly error page-- >
                            < !--a padding to disable MSIE and
                        Chrome friendly error page-- >
                            < !--a padding to disable MSIE and
                        Chrome friendly error page-- >
                            < !--a padding to disable MSIE and
                        Chrome friendly error page-- >));
        return 0;
    });

    server->start();
}

int main(int argc, char **argv)
{
    fatdog::IOManager iom("aoaoao~", 1, true);
    woker.reset(new fatdog::IOManager("worker", 3, false));
    iom.schedule(run);
    return 0;
}
