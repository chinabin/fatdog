#include "../fatdog/tcp_server.h"
#include "../fatdog/iomanager.h"
#include "../fatdog/log.h"

fatdog::Logger::ptr g_logger = FATDOG_LOG_ROOT();

void run()
{
    auto addr = fatdog::Address::LookupAny("0.0.0.0:8033");
    //auto addr2 = fatdog::UnixAddress::ptr(new fatdog::UnixAddress("/tmp/unix_addr"));
    std::vector<fatdog::Address::ptr> addrs;
    addrs.push_back(addr);
    //addrs.push_back(addr2);

    fatdog::TcpServer::ptr tcp_server(new fatdog::TcpServer);
    std::vector<fatdog::Address::ptr> fails;
    while (!tcp_server->bind(addrs, fails))
    {
        sleep(2);
    }
    tcp_server->start();
}
int main(int argc, char **argv)
{
    fatdog::IOManager iom("lala", 2);
    iom.schedule(run);
    return 0;
}
