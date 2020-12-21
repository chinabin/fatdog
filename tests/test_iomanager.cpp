#include "../fatdog/iomanager.h"
#include "../fatdog/log.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <sys/epoll.h>
#include <string.h>

fatdog::Logger::ptr g_logger = FATDOG_LOG_ROOT();

int sock = 0;

void test_fiber() {
    FATDOG_LOG_INFO(g_logger) << "test_fiber sock=" << sock;

    //sleep(3);

    //close(sock);
    //sylar::IOManager::GetThis()->cancelAll(sock);

    sock = socket(AF_INET, SOCK_STREAM, 0);
    fcntl(sock, F_SETFL, O_NONBLOCK);

    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(80);
    inet_pton(AF_INET, "14.215.177.39", &addr.sin_addr.s_addr);

    if(!connect(sock, (const sockaddr*)&addr, sizeof(addr))) {
    } else if(errno == EINPROGRESS) {
        FATDOG_LOG_INFO(g_logger) << "add event errno=" << errno << " " << strerror(errno);
        fatdog::IOManager::GetThis()->addEvent(sock, fatdog::IOManager::READ, [](){
            FATDOG_LOG_INFO(g_logger) << "read callback";
        });
        fatdog::IOManager::GetThis()->addEvent(sock, fatdog::IOManager::WRITE, [](){
            FATDOG_LOG_INFO(g_logger) << "write callback";
            //close(sock);
            fatdog::IOManager::GetThis()->cancelEvent(sock, fatdog::IOManager::READ);
            close(sock);
        });
    } else {
        FATDOG_LOG_INFO(g_logger) << "else " << errno << " " << strerror(errno);
    }

}

void test1() {
    std::cout << "EPOLLIN=" << EPOLLIN
              << " EPOLLOUT=" << EPOLLOUT << std::endl;
    fatdog::IOManager iom("guguji", 1, false);
    iom.schedule(&test_fiber);
}

int main(int argc, char** argv) {
    test1();
    return 0;
}
