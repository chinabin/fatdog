#include <iostream>
#include "../fatdog/http/http_connection.h"
#include "../fatdog/log.h"
#include "../fatdog/iomanager.h"
#include <fstream>

static fatdog::Logger::ptr g_logger = FATDOG_LOG_ROOT();

void test_pool() {
    fatdog::http::HttpConnectionPool::ptr pool(new fatdog::http::HttpConnectionPool(
                "www.fatdog.top", "", 80, 10, 1000 * 30, 5));

    fatdog::IOManager::GetThis()->addTimer(1000, [pool](){
            auto r = pool->doGet("/", 300);
            FATDOG_LOG_INFO(g_logger) << r->toString();
    }, true);
}

void run() {
    fatdog::Address::ptr addr = fatdog::Address::LookupAnyIPAddress("www.fatdog.top:80");
    if(!addr) {
        FATDOG_LOG_INFO(g_logger) << "get addr error";
        return;
    }

    fatdog::Socket::ptr sock = fatdog::Socket::CreateTCP(addr);
    bool rt = sock->connect(addr);
    if(!rt) {
        FATDOG_LOG_INFO(g_logger) << "connect " << *addr << " failed";
        return;
    }

    fatdog::http::HttpConnection::ptr conn(new fatdog::http::HttpConnection(sock));
    fatdog::http::HttpRequest::ptr req(new fatdog::http::HttpRequest);
    req->setPath("/blog/");
    req->setHeader("host", "www.fatdog.top");
    FATDOG_LOG_INFO(g_logger) << "req:" << std::endl
        << *req;

    conn->sendRequest(req);
    auto rsp = conn->recvResponse();

    if(!rsp) {
        FATDOG_LOG_INFO(g_logger) << "recv response error";
        return;
    }
    FATDOG_LOG_INFO(g_logger) << "rsp:" << std::endl
        << *rsp;

    std::ofstream ofs("rsp.dat");
    ofs << *rsp;

    FATDOG_LOG_INFO(g_logger) << "=========================";

    auto r = fatdog::http::HttpConnection::DoGet("http://www.fatdog.top/blog/", 300);
    FATDOG_LOG_INFO(g_logger) << "result=" << r->result
        << " error=" << r->error
        << " rsp=" << (r->response ? r->response->toString() : "");

    FATDOG_LOG_INFO(g_logger) << "=========================";
    test_pool();
}

int main(int argc, char** argv) {
    fatdog::IOManager iom(2);
    iom.schedule(run);
    return 0;
}
