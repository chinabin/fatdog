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

void test2() {
    fatdog::IPAddress::ptr addr = fatdog::Address::LookupAnyIPAddress("www.baidu.com:80");
    if(addr) {
        FATDOG_LOG_INFO(g_looger) << "get address: " << addr->toString();
    } else {
        FATDOG_LOG_ERROR(g_looger) << "get address fail";
        return;
    }

    fatdog::Socket::ptr sock = fatdog::Socket::CreateTCP(addr);
    if(!sock->connect(addr)) {
        FATDOG_LOG_ERROR(g_looger) << "connect " << addr->toString() << " fail";
        return;
    } else {
        FATDOG_LOG_INFO(g_looger) << "connect " << addr->toString() << " connected";
    }

    uint64_t ts = fatdog::GetCurrentUS();
    for(size_t i = 0; i < 10000000000ul; ++i) {
        if(int err = sock->getError()) {
            FATDOG_LOG_INFO(g_looger) << "err=" << err << " errstr=" << strerror(err);
            break;
        }

        //struct tcp_info tcp_info;
        //if(!sock->getOption(IPPROTO_TCP, TCP_INFO, tcp_info)) {
        //    FATDOG_LOG_INFO(g_looger) << "err";
        //    break;
        //}
        //if(tcp_info.tcpi_state != TCP_ESTABLISHED) {
        //    FATDOG_LOG_INFO(g_looger)
        //            << " state=" << (int)tcp_info.tcpi_state;
        //    break;
        //}
        static int batch = 10000000;
        if(i && (i % batch) == 0) {
            uint64_t ts2 = fatdog::GetCurrentUS();
            FATDOG_LOG_INFO(g_looger) << "i=" << i << " used: " << ((ts2 - ts) * 1.0 / batch) << " us";
            ts = ts2;
        }
    }
}

int main(int argc, char **argv)
{
    fatdog::IOManager iom("wang");
    // iom.schedule(&test_socket);
    iom.schedule(&test2);
    return 0;
}
