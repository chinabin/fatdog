#include "../fatdog/tcp_server.h"
#include "../fatdog/log.h"
#include "../fatdog/iomanager.h"
#include "../fatdog/bytearray.h"
#include "../fatdog/address.h"

static fatdog::Logger::ptr g_logger = FATDOG_LOG_ROOT();

class EchoServer : public fatdog::TcpServer
{
public:
    EchoServer(int type);
    void handleClient(fatdog::Socket::ptr client);

private:
    int m_type = 0;
};

EchoServer::EchoServer(int type)
    : m_type(type)
{
}

void EchoServer::handleClient(fatdog::Socket::ptr client)
{
    FATDOG_LOG_INFO(g_logger) << "handleClient " << *client;
    fatdog::ByteArray::ptr ba(new fatdog::ByteArray);
    while (true)
    {
        ba->clear();
        std::vector<iovec> iovs;
        ba->getWriteBuffers(iovs, 1024);

        int rt = client->recv(&iovs[0], iovs.size());
        if (rt == 0)
        {
            FATDOG_LOG_INFO(g_logger) << "client close: " << *client;
            break;
        }
        else if (rt < 0)
        {
            FATDOG_LOG_INFO(g_logger) << "client error rt=" << rt
                                      << " errno=" << errno << " errstr=" << strerror(errno);
            break;
        }
        ba->setPosition(ba->getPosition() + rt);
        ba->setPosition(0);
        //FATDOG_LOG_INFO(g_logger) << "recv rt=" << rt << " data=" << std::string((char*)iovs[0].iov_base, rt);
        if (m_type == 1)
        {                                //text
            std::cout << ba->toString(); // << std::endl;
        }
        else
        {
            std::cout << ba->toHexString(); // << std::endl;
        }
        std::cout.flush();
    }
}

int type = 1;

void run()
{
    FATDOG_LOG_INFO(g_logger) << "server type=" << type;
    EchoServer::ptr es(new EchoServer(type));
    auto addr = fatdog::Address::LookupAny("0.0.0.0:8020");
    while (!es->bind(addr))
    {
        sleep(2);
    }
    es->start();
}

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        FATDOG_LOG_INFO(g_logger) << "used as[" << argv[0] << " -t] or [" << argv[0] << " -b]";
        return 0;
    }

    if (!strcmp(argv[1], "-b"))
    {
        type = 2;
    }

    fatdog::IOManager iom("giao", 2);
    iom.schedule(run);
    return 0;
}
