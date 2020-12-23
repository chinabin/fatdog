#include "../fatdog/socket.h"
#include "../fatdog/iomanager.h"
#include "../fatdog/log.h"
#include "../fatdog/address.h"

static fatdog::Logger::ptr g_looger = FATDOG_LOG_ROOT();

void test_socket()
{
    //std::vector<fatdog::Address::ptr> addrs;
    //fatdog::Address::Lookup(addrs, "www.baidu.com", AF_INET);
    //fatdog::IPAddress::ptr addr;
    //for(auto& i : addrs) {
    //    FATDOG_LOG_INFO(g_looger) << i->toString();
    //    addr = std::dynamic_pointer_cast<fatdog::IPAddress>(i);
    //    if(addr) {
    //        break;
    //    }
    //}
    fatdog::IPAddress::ptr addr = fatdog::Address::LookupAnyIPAddress("www.baidu.com");
    if (addr)
    {
        FATDOG_LOG_INFO(g_looger) << "get address: " << addr->toString();
    }
    else
    {
        FATDOG_LOG_ERROR(g_looger) << "get address fail";
        return;
    }

    fatdog::Socket::ptr sock = fatdog::Socket::CreateTCP(addr);
    addr->setPort(80);
    FATDOG_LOG_INFO(g_looger) << "addr=" << addr->toString();
    if (!sock->connect(addr))
    {
        FATDOG_LOG_ERROR(g_looger) << "connect " << addr->toString() << " fail";
        return;
    }
    else
    {
        FATDOG_LOG_INFO(g_looger) << "connect " << addr->toString() << " connected";
    }

    const char buff[] = "GET / HTTP/1.0\r\n\r\n";
    int rt = sock->send(buff, sizeof(buff));
    if (rt <= 0)
    {
        FATDOG_LOG_INFO(g_looger) << "send fail rt=" << rt;
        return;
    }

    std::string buffs;
    buffs.resize(4096);
    rt = sock->recv(&buffs[0], buffs.size());

    if (rt <= 0)
    {
        FATDOG_LOG_INFO(g_looger) << "recv fail rt=" << rt;
        return;
    }

    buffs.resize(rt);
    FATDOG_LOG_INFO(g_looger) << buffs;
}

int main(int argc, char **argv)
{
    fatdog::IOManager iom("wang");
    iom.schedule(&test_socket);
    return 0;
}
