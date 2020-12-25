#ifndef __FATDOG_ADDRESS_H__
#define __FATDOG_ADDRESS_H__

#include <memory>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <vector>
#include <map>

namespace fatdog
{

    class IPAddress;
    class Address
    {
    public:
        typedef std::shared_ptr<Address> ptr;

        static Address::ptr Create(const sockaddr *addr, socklen_t addrlen);
        // 通过host地址返回对应条件的所有Address
        static bool Lookup(std::vector<Address::ptr> &result, const std::string &host, int family = AF_INET, int type = 0, int protocol = 0);
        // 通过host地址返回对应条件的任意Address
        static Address::ptr LookupAny(const std::string &host, int family = AF_INET, int type = 0, int protocol = 0);
        static std::shared_ptr<IPAddress> LookupAnyIPAddress(const std::string &host,
                                                             int family = AF_INET, int type = 0, int protocol = 0);

        // 返回本机所有网卡的<网卡名, 地址, 子网掩码位数>
        // family 协议族(AF_INT, AF_UNIX)
        static bool GetInterfaceAddresses(std::multimap<std::string, std::pair<Address::ptr, uint32_t>> &result, int family = AF_INET);
        // 获取指定网卡的<地址, 子网掩码位数>
        // iface 网卡名称, family 协议族(AF_INT, AF_UNIX)
        static bool GetInterfaceAddresses(std::vector<std::pair<Address::ptr, uint32_t>> &result, const std::string &iface, int family = AF_INET);

        virtual ~Address() {}

        int getFamily() const;

        virtual const sockaddr *getAddr() const = 0;
        virtual sockaddr *getAddr() = 0;
        virtual socklen_t getAddrLen() const = 0;

        virtual std::ostream &insert(std::ostream &os) const = 0;
        std::string toString() const;

        bool operator<(const Address &rhs) const;
        bool operator==(const Address &rhs) const;
        bool operator!=(const Address &rhs) const;
    };

    class IPAddress : public Address
    {
    public:
        typedef std::shared_ptr<IPAddress> ptr;

        // 通过域名、IP、服务器名创建IPAddress
        static IPAddress::ptr Create(const char *address, uint16_t port = 0);

        // 获取该地址的广播地址，prefix_len 子网掩码位数
        virtual IPAddress::ptr broadcastAddress(uint32_t prefix_len) = 0;
        // 获取该地址的网段, prefix_len 子网掩码位数
        virtual IPAddress::ptr networdAddress(uint32_t prefix_len) = 0;
        // 获取子网掩码地址, prefix_len 子网掩码位数
        virtual IPAddress::ptr subnetMask(uint32_t prefix_len) = 0;

        virtual uint32_t getPort() const = 0;
        virtual void setPort(uint16_t v) = 0;
    };

    class IPv4Address : public IPAddress
    {
    public:
        typedef std::shared_ptr<IPv4Address> ptr;

        // 使用点分十进制地址创建IPv4Address
        static IPv4Address::ptr Create(const char *address, uint16_t port = 0);

        IPv4Address(const sockaddr_in &address);
        // 通过二进制地址构造IPv4Address
        IPv4Address(uint32_t address = INADDR_ANY, uint16_t port = 0);

        const sockaddr *getAddr() const override;
        sockaddr *getAddr() override;
        socklen_t getAddrLen() const override;
        std::ostream &insert(std::ostream &os) const override;

        IPAddress::ptr broadcastAddress(uint32_t prefix_len) override;
        IPAddress::ptr networdAddress(uint32_t prefix_len) override;
        IPAddress::ptr subnetMask(uint32_t prefix_len) override;
        uint32_t getPort() const override;
        void setPort(uint16_t v) override;

    private:
        sockaddr_in m_addr;
    };

    class UnixAddress : public Address
    {
    public:
        typedef std::shared_ptr<UnixAddress> ptr;
        UnixAddress();
        // 通过路径构造UnixAddress
        UnixAddress(const std::string &path);

        const sockaddr *getAddr() const override;
        sockaddr *getAddr() override;
        socklen_t getAddrLen() const override;
        void setAddrLen(uint32_t v);
        std::ostream &insert(std::ostream &os) const override;

    private:
        sockaddr_un m_addr;
        socklen_t m_length;
    };

    class UnknownAddress : public Address
    {
    public:
        typedef std::shared_ptr<UnknownAddress> ptr;
        UnknownAddress(int family);
        UnknownAddress(const sockaddr &addr);
        const sockaddr *getAddr() const override;
        sockaddr *getAddr() override;
        socklen_t getAddrLen() const override;
        std::ostream &insert(std::ostream &os) const override;

    private:
        sockaddr m_addr;
    };

    std::ostream &operator<<(std::ostream &os, const Address &addr);

} // namespace fatdog

#endif
