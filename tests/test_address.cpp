#include "../fatdog/address.h"
#include "../fatdog/log.h"

fatdog::Logger::ptr g_logger = FATDOG_LOG_ROOT();

void test() {
    std::vector<fatdog::Address::ptr> addrs;

    FATDOG_LOG_INFO(g_logger) << "begin";
    bool v = fatdog::Address::Lookup(addrs, "www.baidu.com");
    FATDOG_LOG_INFO(g_logger) << "end";
    if(!v) {
        FATDOG_LOG_ERROR(g_logger) << "lookup fail";
        return;
    }

    for(size_t i = 0; i < addrs.size(); ++i) {
        FATDOG_LOG_INFO(g_logger) << i << " - " << addrs[i]->toString();
    }
}

void test_iface() {
    std::multimap<std::string, std::pair<fatdog::Address::ptr, uint32_t> > results;

    bool v = fatdog::Address::GetInterfaceAddresses(results);
    if(!v) {
        FATDOG_LOG_ERROR(g_logger) << "GetInterfaceAddresses fail";
        return;
    }

    for(auto& i: results) {
        FATDOG_LOG_INFO(g_logger) << i.first << " - " << i.second.first->toString() << " - "
            << i.second.second;
    }
}

void test_ipv4() {
    // auto addr = fatdog::IPAddress::Create("www.baidu.com");     // 因为设置了 AI_NUMERICSERV 标志并且 servname 不是一个数字化的端口名字符串，会产生 EAI_NONAME 错误
    auto addr = fatdog::IPAddress::Create("127.0.0.8");
    if(addr) {
        FATDOG_LOG_INFO(g_logger) << addr->toString();
    }
}

int main(int argc, char** argv) {
    test_ipv4();
    test_iface();
    test();
    return 0;
}
