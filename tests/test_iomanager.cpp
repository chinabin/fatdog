#include "../fatdog/iomanager.h"
#include "../fatdog/log.h"
#include "../fatdog/macro.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <sys/epoll.h>
#include <string.h>

static fatdog::Logger::ptr g_logger = FATDOG_LOG_ROOT();

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

fatdog::TimerManager::Timer::ptr s_timer;
void test_timer() {
    fatdog::IOManager iom("lala", 1);
    FATDOG_LOG_INFO(g_logger) << "add timer";
    s_timer = iom.addTimer(1000, [](){
        static int i = 0;
        FATDOG_LOG_INFO(g_logger) << "hello timer i=" << i;
        if(++i == 3) {
            s_timer->reset(2000, true);
            //s_timer->cancel();
        }
        if(i >= 10)
        {
            s_timer->cancel();
        }
    }, true);
}

int main(int argc, char** argv) {
    FATDOG_LOG_INFO(g_logger) << "let's start";
    // test1();
    test_timer();
    FATDOG_LOG_INFO(g_logger) << "let's end";
    return 0;
}
