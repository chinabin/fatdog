#include <iostream>
#include <tuple>
#include <string>
#include <vector>
#include <stdarg.h>

#include "../fatdog/log.h"

void init(std::string m_pattern)
{
    //str, format, type
    std::vector<std::tuple<std::string, std::string, int>> vec;
    std::string nstr;
    for (size_t i = 0; i < m_pattern.size(); ++i)
    {
        if (m_pattern[i] != '%')
        {
            nstr.append(1, m_pattern[i]);
            continue;
        }

        if ((i + 1) < m_pattern.size())
        {
            if (m_pattern[i + 1] == '%')
            {
                nstr.append(1, '%');
                continue;
            }
        }

        size_t n = i + 1;
        int fmt_status = 0;
        size_t fmt_begin = 0;

        std::string str;
        std::string fmt;
        while (n < m_pattern.size())
        {
            if (!fmt_status && (!isalpha(m_pattern[n]) && m_pattern[n] != '{' && m_pattern[n] != '}'))
            {
                str = m_pattern.substr(i + 1, n - i - 1);
                break;
            }
            if (fmt_status == 0)
            {
                if (m_pattern[n] == '{')
                {
                    str = m_pattern.substr(i + 1, n - i - 1);
                    //std::cout << "*" << str << std::endl;
                    fmt_status = 1; //解析格式
                    fmt_begin = n;
                    ++n;
                    continue;
                }
            }
            else if (fmt_status == 1)
            {
                if (m_pattern[n] == '}')
                {
                    fmt = m_pattern.substr(fmt_begin + 1, n - fmt_begin - 1);
                    //std::cout << "#" << fmt << std::endl;
                    fmt_status = 0;
                    ++n;
                    break;
                }
            }
            ++n;
            if (n == m_pattern.size())
            {
                if (str.empty())
                {
                    str = m_pattern.substr(i + 1);
                }
            }
        }

        if (fmt_status == 0)
        {
            if (!nstr.empty())
            {
                vec.push_back(std::make_tuple(nstr, std::string(), 0));
                nstr.clear();
            }
            vec.push_back(std::make_tuple(str, fmt, 1));
            i = n - 1;
        }
        else if (fmt_status == 1)
        {
            std::cout << "pattern parse error: " << m_pattern << " - " << m_pattern.substr(i) << std::endl;
            vec.push_back(std::make_tuple("<<pattern_error>>", fmt, 0));
        }
    }

    if (!nstr.empty())
    {
        vec.push_back(std::make_tuple(nstr, "", 0));
    }

    for (auto &i : vec)
    {
        std::cout << "(" << std::get<0>(i) << ") - (" << std::get<1>(i) << ") - (" << std::get<2>(i) << ")" << std::endl;
    }
    //std::cout << m_items.size() << std::endl;
}

void fun(std::string strPattern)
{
    std::string nstr;
    std::vector<std::tuple<std::string, std::string, int>> vec;
    for (size_t i = 0; i < strPattern.size(); ++i)
    {
        char c = strPattern[i];
        if (c != '%')
        {
            size_t tmp = strPattern.find("%", i);
            nstr += strPattern.substr(i, tmp - i);
            i = tmp - 1;
            continue;
        }

        if ((i + 1) < strPattern.size())
        {
            c = strPattern[i + 1];
            if (c == '%')
            {
                nstr.append(1, c);
                i++;
                continue;
            }
            else if (!isalpha(c))
            {
                size_t tmp = strPattern.find("%", i + 1);
                nstr += strPattern.substr(i, tmp - (i + 1) + 1);
                i = tmp - 1;
                continue;
            }
        }

        /*
         logic above is:
            is '%' ?:
                yes: behind one is '%' ?:
                    yes: add '%'
                    no: behind one is alpha ?:
                        yes: parse with below logic
                        no: add character until get '%'
                no: add character until get '%'
        */

        size_t n = i + 1;        // for strPattern[i] == '%'
        bool fmt_status = false; // means parse betweent '{' and '}'
        size_t fmt_begin = 0;    // position behind '{'
        std::string str;         // keep character behind '%'
        std::string fmt;         // keep string between '{' and '}'
        for (; n < strPattern.size(); ++n)
        {
            c = strPattern[n];

            if (c == '{' && !fmt_status)
            {
                str = strPattern.substr(i + 1, n - (i + 1));
                fmt_begin = n + 1;
                fmt_status = true;
                continue;
            }
            else if (c == '}' && fmt_status == true)
            {
                fmt = strPattern.substr(fmt_begin, n - fmt_begin);
                i = n;
                fmt_status = false;
                break;
            }

            if ((n - i >= 2 || n == strPattern.size() - 1) && !fmt_status)
            {
                str = strPattern.substr(i + 1, 1);
                i += 1;
                break;
            }
        }

        if (fmt_status == false)
        {
            if (!nstr.empty())
            {
                vec.push_back(std::make_tuple(nstr, "", 0));
                nstr.clear();
            }
            vec.push_back(std::make_tuple(str, fmt, 1));
        }
        else
        {
            size_t tmp = strPattern.find("%", i + 1);
            nstr += strPattern.substr(i, tmp - (i + 1) + 1);
            std::cout << "pattern wrong: " << strPattern << " - " << nstr << std::endl;
            i = tmp - 1;
            vec.push_back(std::make_tuple(nstr, fmt, 0));
            nstr.clear();
        }
    }

    for (const auto &i : vec)
    {
        std::cout << "(" << std::get<0>(i) << ") - (" << std::get<1>(i) << ") - (" << std::get<2>(i) << ")" << std::endl;
    }
}

// v0.01 log
void test1()
{
    fatdog::Logger::ptr logger(new fatdog::Logger);

    fatdog::StdoutLogAppender::ptr stdout_appender(new fatdog::StdoutLogAppender);
    logger->addAppender(stdout_appender);
    fatdog::FileLogAppender::ptr file_appender(new fatdog::FileLogAppender("log.txt"));
    logger->addAppender(file_appender);

    fatdog::LogFormatter::ptr formatter(new fatdog::LogFormatter("%d{%Y-%m-%d %H:%M:%S}%T%t%T%N%T%F%T[%p]%T[%c]%T%f:%l%T%m%n"));
    stdout_appender->setFormatter(formatter);
    file_appender->setFormatter(formatter);

    std::cout << "begin" << std::endl;
    {
        fatdog::LogEvent::ptr event(new fatdog::LogEvent(logger, fatdog::LogLevel::DEBUG, __FILE__, __LINE__, 0, 1, 2, time(0), "haha"));
        event->getSS() << "this is a test";
        logger->info(event);
    }
    {
        fatdog::LogEvent::ptr event(new fatdog::LogEvent(logger, fatdog::LogLevel::DEBUG, __FILE__, __LINE__, 0, 1, 2, time(0), "haha"));
        event->getSS() << "hehehaha";
        logger->info(event);
    }
    {
        fatdog::LogEvent::ptr event(new fatdog::LogEvent(logger, fatdog::LogLevel::DEBUG, __FILE__, __LINE__, 0, 1, 2, time(0), "haha"));
        event->getSS() << "lalala";
        logger->info(event);
    }
    std::cout << "end" << std::endl;
}

// v0.02 log
void test2()
{
    fatdog::Logger::ptr logger(new fatdog::Logger);

    fatdog::StdoutLogAppender::ptr stdout_appender(new fatdog::StdoutLogAppender);
    logger->addAppender(stdout_appender);
    fatdog::FileLogAppender::ptr file_appender(new fatdog::FileLogAppender("log.txt"));
    logger->addAppender(file_appender);

    // we don't need this anymore, we can use default formatter
    // fatdog::LogFormatter::ptr formatter(new fatdog::LogFormatter("%d{%Y-%m-%d %H:%M:%S}%T%t%T%N%T%F%T[%p]%T[%c]%T%f:%l%T%m%n"));
    // stdout_appender->setFormatter(formatter);
    // file_appender->setFormatter(formatter);

    std::cout << "begin" << std::endl;
    {
        fatdog::LogEvent::ptr event(new fatdog::LogEvent(logger, fatdog::LogLevel::DEBUG, __FILE__, __LINE__, 0, 1, 2, time(0), "haha"));
        event->getSS() << "this is a test";
        logger->info(event);
    }
    {
        fatdog::LogEvent::ptr event(new fatdog::LogEvent(logger, fatdog::LogLevel::DEBUG, __FILE__, __LINE__, 0, 1, 2, time(0), "haha"));
        event->getSS() << "hehehaha";
        logger->info(event);
    }
    {
        fatdog::LogEvent::ptr event(new fatdog::LogEvent(logger, fatdog::LogLevel::DEBUG, __FILE__, __LINE__, 0, 1, 2, time(0), "haha"));
        event->getSS() << "lalala";
        logger->info(event);
    }
    std::cout << "end" << std::endl;
}

// v0.02 log
void test3()
{
    fatdog::Logger::ptr logger(new fatdog::Logger);

    fatdog::StdoutLogAppender::ptr stdout_appender(new fatdog::StdoutLogAppender);
    logger->addAppender(stdout_appender);
    fatdog::FileLogAppender::ptr file_appender(new fatdog::FileLogAppender("log.txt"));
    logger->addAppender(file_appender);

    // we don't need this anymore, we can use default formatter
    // fatdog::LogFormatter::ptr formatter(new fatdog::LogFormatter("%d{%Y-%m-%d %H:%M:%S}%T%t%T%N%T%F%T[%p]%T[%c]%T%f:%l%T%m%n"));
    // stdout_appender->setFormatter(formatter);
    // file_appender->setFormatter(formatter);

    std::cout << "begin" << std::endl;
    {
        fatdog::LogEvent::ptr event(new fatdog::LogEvent(logger, fatdog::LogLevel::DEBUG, __FILE__, __LINE__, 0, 1, 2, time(0), "haha"));
        // event->getSS() << "this is a test";
        event->format("this %s a test %d %f", "is", 123, 5.67);
        logger->info(event);
    }

    std::cout << "end" << std::endl;
}

void test4()
{
    // use macro
    fatdog::Logger::ptr logger(new fatdog::Logger);
    FATDOG_LOG_INFO(logger) << "lalala";
    FATDOG_LOG_FMT_INFO(logger, "a a a %s %d %f", "miao", 1, 2.34);
}

int main()
{
    // std::string str = "%d{%Y-%m-%d %H:%M:%S}%T%t%T%N%T%F%T[%p]%T[%c]%T%f:%l%T%m%n";
    // // std::string str = "%dm%n";
    // // init(str);
    // str = "[[[%d{%Y-%m-%d %H:%M:%S}%T{%t%T%N%T%F%T[%p]%T[%c]%T%f:%l%T%m%n";
    // fun(str);

    // test1();
    // test2();
    // test3();
    test4();

    return 0;
}