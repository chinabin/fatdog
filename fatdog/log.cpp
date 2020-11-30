#include "log.h"

#include <iostream>
#include <map>
#include <functional>
#include <tuple>

namespace fatdog
{

    std::string LogLevel::ToString(Level level)
    {
        switch (level)
        {
#define XX(name)         \
    case LogLevel::name: \
        return #name;

            XX(DEBUG);
            XX(INFO);
            XX(WARN);
            XX(ERROR);
            XX(FATAL);

#undef XX
        default:
            return "UNKNOWN";
        }
        return "UNKNOWN";
    }

    LogLevel::Level FromString(const std::string &str)
    {
#define XX(level, name) \
    if (str == #name)   \
        return LogLevel::level;

        XX(DEBUG, debug);
        XX(INFO, info);
        XX(WARN, warn);
        XX(ERROR, error);
        XX(FATAL, fatal);

        XX(DEBUG, DEBUG);
        XX(INFO, INFO);
        XX(WARN, WARN);
        XX(ERROR, ERROR);
        XX(FATAL, FATAL);

#undef XX
    }

    LogEvent::LogEvent(std::shared_ptr<Logger> logger, LogLevel::Level level, const char *file, int32_t line, uint32_t elapse, uint32_t thread_id, uint32_t fiber_id, uint64_t time, const std::string &thread_name)
        : m_filename(file), m_line(line), m_elapse(elapse), m_threadID(thread_id), m_fiberID(fiber_id), m_time(time), m_threadName(thread_name), m_logger(logger), m_level(level)
    {
    }

    Logger::~Logger()
    {
    }

    void Logger::debug(LogEvent::ptr event)
    {
        log(LogLevel::DEBUG, event);
    }

    void Logger::info(LogEvent::ptr event)
    {
        log(LogLevel::INFO, event);
    }

    void Logger::warn(LogEvent::ptr event)
    {
        log(LogLevel::WARN, event);
    }

    void Logger::error(LogEvent::ptr event)
    {
        log(LogLevel::ERROR, event);
    }

    void Logger::fatal(LogEvent::ptr event)
    {
        log(LogLevel::FATAL, event);
    }

    void Logger::log(LogLevel::Level level, LogEvent::ptr event)
    {
        if (level >= m_level)
        {
            auto self = shared_from_this();
            for (auto &it : m_appenders)
            {
                it->log(self, level, event);
            }
        }
    }

    void Logger::addAppender(std::shared_ptr<LogAppender> appender)
    {
        m_appenders.push_back(appender);
    }

    void Logger::delAppender(std::shared_ptr<LogAppender> appender)
    {
        for (auto it = m_appenders.begin(); it != m_appenders.end();)
        {
            if (*it == appender)
            {
                it = m_appenders.erase(it);
                break;
            }
            else
            {
                ++it;
            }
        }
    }

    void Logger::clearAppenders()
    {
        m_appenders.clear();
    }

    void Logger::setFormatter(LogFormatter::ptr formatter)
    {
        m_formatter = formatter;
    }

    void StdoutLogAppender::log(Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event)
    {
        std::cout << m_formatter->format(logger, level, event);
    }

    FileLogAppender::FileLogAppender(const std::string &filename)
        : m_filename(filename)
    {
        reopen();
    }

    void FileLogAppender::log(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event)
    {
        if (level >= m_level)
        {
            uint64_t now = time(0);
            if (now != m_lastTime)
            {
                reopen();
                m_lastTime = now;
            }

            if (!(m_filestream << m_formatter->format(logger, level, event)))
            {
                std::cout << "error" << std::endl;
            }
        }
    }

    bool FileLogAppender::reopen()
    {
        if (m_filestream.is_open())
        {
            m_filestream.close();
        }
        m_filestream.open(m_filename, std::ios::app);
        return !!m_filestream;
    }

    class MessageFormatItem : public LogFormatter::FormatItem
    {
    public:
        MessageFormatItem(const std::string &str = "") {}
        void format(std::ostream &os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override
        {
            os << event->getContent();
        }
    };

    class LevelFormatItem : public LogFormatter::FormatItem
    {
    public:
        LevelFormatItem(const std::string &str = "") {}
        void format(std::ostream &os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override
        {
            os << LogLevel::ToString(event->getLevel());
        }
    };

    class ElapseFormatItem : public LogFormatter::FormatItem
    {
    public:
        ElapseFormatItem(const std::string &str = "") {}
        void format(std::ostream &os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override
        {
            os << event->getElapse();
        }
    };

    class LogNameFormatItem : public LogFormatter::FormatItem
    {
    public:
        LogNameFormatItem(const std::string &str = "") {}
        void format(std::ostream &os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override
        {
            os << event->getLogger()->getName();
        }
    };

    class ThreadIdFormatItem : public LogFormatter::FormatItem
    {
    public:
        ThreadIdFormatItem(const std::string &str = "") {}
        void format(std::ostream &os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override
        {
            os << event->getThreadId();
        }
    };

    class FiberIdFormatItem : public LogFormatter::FormatItem
    {
    public:
        FiberIdFormatItem(const std::string &str = "") {}
        void format(std::ostream &os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override
        {
            os << event->getFiberId();
        }
    };

    class ThreadNameFormatItem : public LogFormatter::FormatItem
    {
    public:
        ThreadNameFormatItem(const std::string &str = "") {}
        void format(std::ostream &os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override
        {
            os << event->getThreadName();
        }
    };

    class DateTimeFormatItem : public LogFormatter::FormatItem
    {
    public:
        DateTimeFormatItem(const std::string &format = "%Y-%m-%d %H:%M:%S")
            : m_format(format)
        {
            if (m_format.empty())
            {
                m_format = "%Y-%m-%d %H:%M:%S";
            }
        }

        void format(std::ostream &os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override
        {
            struct tm tm;
            time_t time = event->getTime();
            localtime_r(&time, &tm);
            char buf[64];
            strftime(buf, sizeof(buf), m_format.c_str(), &tm);
            os << buf;
        }

    private:
        std::string m_format;
    };

    class FilenameFormatItem : public LogFormatter::FormatItem
    {
    public:
        FilenameFormatItem(const std::string &str = "") {}
        void format(std::ostream &os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override
        {
            os << event->getFile();
        }
    };

    class LineFormatItem : public LogFormatter::FormatItem
    {
    public:
        LineFormatItem(const std::string &str = "") {}
        void format(std::ostream &os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override
        {
            os << event->getLine();
        }
    };

    class NewLineFormatItem : public LogFormatter::FormatItem
    {
    public:
        NewLineFormatItem(const std::string &str = "") {}
        void format(std::ostream &os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override
        {
            os << std::endl;
        }
    };

    class StringFormatItem : public LogFormatter::FormatItem
    {
    public:
        StringFormatItem(const std::string &str)
            : m_string(str) {}
        void format(std::ostream &os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override
        {
            os << m_string;
        }

    private:
        std::string m_string;
    };

    class TabFormatItem : public LogFormatter::FormatItem
    {
    public:
        TabFormatItem(const std::string &str = "") {}
        void format(std::ostream &os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override
        {
            os << "\t";
        }

    private:
        std::string m_string;
    };

    LogFormatter::LogFormatter(const std::string &pattern)
        : m_pattern(pattern)
    {
        init();
    }

    void LogFormatter::init()
    {
        std::string nstr;
        std::vector<std::tuple<std::string, std::string, int>> vec; // str, fmt, type(0 or 1)
        for (size_t i = 0; i < m_pattern.size(); ++i)
        {
            char c = m_pattern[i];
            if (c != '%')
            {
                size_t tmp = m_pattern.find("%", i);
                nstr += m_pattern.substr(i, tmp - i);
                i = tmp - 1;
                continue;
            }

            if ((i + 1) < m_pattern.size())
            {
                c = m_pattern[i + 1];
                if (c == '%')
                {
                    nstr.append(1, c);
                    i++;
                    continue;
                }
                else if (!isalpha(c))
                {
                    size_t tmp = m_pattern.find("%", i + 1);
                    nstr += m_pattern.substr(i, tmp - (i + 1) + 1);
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

            size_t n = i + 1;        // for m_pattern[i] == '%'
            bool fmt_status = false; // means parse betweent '{' and '}'
            size_t fmt_begin = 0;    // position behind '{'
            std::string str;         // keep character behind '%'
            std::string fmt;         // keep string between '{' and '}'
            for (; n < m_pattern.size(); ++n)
            {
                c = m_pattern[n];

                if (c == '{' && !fmt_status)
                {
                    str = m_pattern.substr(i + 1, n - (i + 1));
                    fmt_begin = n + 1;
                    fmt_status = true;
                    continue;
                }
                else if (c == '}' && fmt_status == true)
                {
                    fmt = m_pattern.substr(fmt_begin, n - fmt_begin);
                    i = n;
                    fmt_status = false;
                    break;
                }

                if ((n - i >= 2 || n == m_pattern.size() - 1) && !fmt_status)
                {
                    str = m_pattern.substr(i + 1, 1);
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
                size_t tmp = m_pattern.find("%", i + 1);
                nstr += m_pattern.substr(i, tmp - (i + 1) + 1);
                std::cout << "pattern wrong: " << m_pattern << " - " << nstr << std::endl;
                i = tmp - 1;
                vec.push_back(std::make_tuple(nstr, fmt, 0));
                nstr.clear();
            }
        }

        static std::map<std::string, std::function<FormatItem::ptr(const std::string &str)>> s_format_items = {
#define XX(str, C)                                                               \
    {                                                                            \
#str, [](const std::string &fmt) { return FormatItem::ptr(new C(fmt)); } \
    }

            XX(m, MessageFormatItem),    //m:消息
            XX(p, LevelFormatItem),      //p:日志级别
            XX(r, ElapseFormatItem),     //r:累计毫秒数
            XX(c, LogNameFormatItem),    //c:日志名称
            XX(t, ThreadIdFormatItem),   //t:线程id
            XX(n, NewLineFormatItem),    //n:换行
            XX(d, DateTimeFormatItem),   //d:时间
            XX(f, FilenameFormatItem),   //f:文件名
            XX(l, LineFormatItem),       //l:行号
            XX(T, TabFormatItem),        //T:Tab
            XX(F, FiberIdFormatItem),    //F:协程id
            XX(N, ThreadNameFormatItem), //N:线程名称
#undef XX
        };

        for (auto &i : vec)
        {
            if (std::get<2>(i) == 0)
            {
                m_items.push_back(FormatItem::ptr(new StringFormatItem(std::get<0>(i))));
            }
            else
            {
                auto it = s_format_items.find(std::get<0>(i));
                if (it == s_format_items.end())
                {
                    m_items.push_back(FormatItem::ptr(new StringFormatItem("<<error_format %" + std::get<0>(i) + ">>")));
                }
                else
                {
                    m_items.push_back(it->second(std::get<1>(i)));
                }
            }

            //std::cout << "(" << std::get<0>(i) << ") - (" << std::get<1>(i) << ") - (" << std::get<2>(i) << ")" << std::endl;
        }
        //std::cout << m_items.size() << std::endl;
    }

    std::string LogFormatter::format(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event)
    {
        std::stringstream ss;

        for (auto &it : m_items)
        {
            it->format(ss, logger, level, event);
        }

        return ss.str();
    }

} // namespace fatdog
