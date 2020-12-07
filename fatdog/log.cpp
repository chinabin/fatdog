#include "log.h"

#include <iostream>
#include <map>
#include <functional>
#include <tuple>
#include <stdarg.h>

#include "config.h"

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

    LogLevel::Level LogLevel::FromString(const std::string &str)
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
        return LogLevel::UNKNOWN;
    }

    LogEvent::LogEvent(std::shared_ptr<Logger> logger, LogLevel::Level level, const char *file, int32_t line, uint32_t elapse, uint32_t thread_id, uint32_t fiber_id, uint64_t time, const std::string &thread_name)
        : m_filename(file), m_line(line), m_elapse(elapse), m_threadID(thread_id), m_fiberID(fiber_id), m_time(time), m_threadName(thread_name), m_logger(logger), m_level(level)
    {
    }

    void LogEvent::format(const char *fmt, ...)
    {
        va_list al;
        va_start(al, fmt);

        char *buf = nullptr;
        int len = vasprintf(&buf, fmt, al);
        if (len != -1)
        {
            m_ss << std::string(buf, len);
            free(buf);
        }

        va_end(al);
    }

    LogEventWrapper::~LogEventWrapper()
    {
        m_event->getLogger()->log(m_event->getLevel(), m_event);
    }

    Logger::Logger(const std::string &name, const LogLevel::Level level)
        : m_name(name), m_level(level)
    {
        m_formatter.reset(new LogFormatter("%d{%Y-%m-%d %H:%M:%S}%T%t%T%N%T%F%T[%p]%T[%c]%T%f:%l%T%m%n"));
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
            if (m_appenders.empty())
            {
                addAppender(LogAppender::ptr(new StdoutLogAppender));
            }
            auto self = shared_from_this();
            for (auto &it : m_appenders)
            {
                it->log(self, level, event);
            }
        }
    }

    void Logger::addAppender(std::shared_ptr<LogAppender> appender)
    {
        if (appender->getFormatter() == nullptr)
            appender->setFormatter(m_formatter);
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

    void Logger::setFormatter(const std::string &val)
    {
        fatdog::LogFormatter::ptr new_val(new fatdog::LogFormatter(val));
        if (new_val->isError())
        {
            std::cout << "Logger setFormatter name=" << m_name
                      << " value=" << val << " invalid formatter"
                      << std::endl;
            return;
        }
        setFormatter(new_val);
    }

    std::string Logger::toYamlString()
    {
        YAML::Node node;
        node["name"] = m_name;
        node["level"] = LogLevel::ToString(m_level);

        if (m_formatter)
        {
            node["formatter"] = m_formatter->getPattern();
        }

        for (auto &i : m_appenders)
        {
            node["appenders"].push_back(YAML::Load(i->toYamlString()));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }

    void StdoutLogAppender::log(Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event)
    {
        std::cout << m_formatter->format(logger, level, event);
    }

    std::string StdoutLogAppender::toYamlString()
    {
        YAML::Node node;
        node["type"] = "StdoutLogAppender";
        node["level"] = LogLevel::ToString(m_level);

        if (m_formatter)
        {
            node["formatter"] = m_formatter->getPattern();
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }

    FileLogAppender::FileLogAppender(const std::string &filename, LogLevel::Level level)
        : LogAppender(level), m_filename(filename)
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

    std::string FileLogAppender::toYamlString()
    {
        YAML::Node node;
        node["type"] = "FileLogAppender";
        node["file"] = m_filename;
        node["level"] = LogLevel::ToString(m_level);

        if (m_formatter)
        {
            node["formatter"] = m_formatter->getPattern();
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
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
                m_error = true;
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

    LoggerManager::LoggerManager()
    {
        m_root.reset(new Logger("root", LogLevel::INFO));
        m_root->addAppender(LogAppender::ptr(new StdoutLogAppender(LogLevel::INFO)));

        m_loggers[m_root->getName()] = m_root;
    }

    Logger::ptr LoggerManager::getLogger(const std::string &name)
    {
        auto t = m_loggers.find(name);
        if (t != m_loggers.end())
        {
            return t->second;
        }

        Logger::ptr logger(new Logger(name));
        m_loggers[name] = logger;

        return logger;
    }

    std::string LoggerManager::toYamlString()
    {
        YAML::Node node;
        for (auto &i : m_loggers)
        {
            node.push_back(YAML::Load(i.second->toYamlString()));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }

    struct LogAppenderDefine
    {
        int type = 0; //1 File, 2 Stdout
        LogLevel::Level level = LogLevel::UNKNOWN;
        std::string formatter;
        std::string file;

        bool operator==(const LogAppenderDefine &oth) const
        {
            return type == oth.type && level == oth.level && formatter == oth.formatter && file == oth.file;
        }
    };

    struct LogDefine
    {
        std::string name;
        LogLevel::Level level = LogLevel::UNKNOWN;
        std::string formatter;
        std::vector<LogAppenderDefine> appenders;

        bool operator==(const LogDefine &oth) const
        {
            return name == oth.name && level == oth.level && formatter == oth.formatter && appenders == appenders;
        }

        bool operator<(const LogDefine &oth) const
        {
            return name < oth.name;
        }
    };

    template <>
    class LexicalCast<std::string, std::set<LogDefine>>
    {
    public:
        std::set<LogDefine> operator()(const std::string &v)
        {
            YAML::Node node = YAML::Load(v);
            std::set<LogDefine> vec;

            for (int i = 0; i < node.size(); ++i)
            {
                auto n = node[i];

                LogDefine ld;
                ld.name = n["name"].as<std::string>();
                ld.level = LogLevel::FromString(n["level"].as<std::string>());
                ld.formatter = n["formatter"].as<std::string>();

                for (int j = 0; j < n["appender"].size(); ++j)
                {
                    LogAppenderDefine lad;
                    auto na = n["appender"][j];
                    std::string appenderType = na["type"].as<std::string>();
                    if (appenderType == "FileLogAppender")
                    {
                        lad.type = 1;
                        lad.level = LogLevel::FromString(na["level"].as<std::string>());
                        lad.formatter = na["formatter"].as<std::string>();
                        lad.file = na["file"].as<std::string>();
                    }
                    else if (appenderType == "StdoutLogAppender")
                    {
                        lad.type = 2;
                        lad.level = LogLevel::FromString(na["level"].as<std::string>());
                        lad.formatter = na["formatter"].as<std::string>();
                    }
                    ld.appenders.push_back(lad);
                }
                vec.insert(ld);
            }

            return vec;
        }
    };

    template <>
    class LexicalCast<std::set<LogDefine>, std::string>
    {
    public:
        std::string operator()(const std::set<LogDefine> &v)
        {
            YAML::Node node;
            for (auto &i : v)
            {
                YAML::Node n;
                n["name"] = i.name;
                n["level"] = LogLevel::ToString(i.level);
                n["formatter"] = i.formatter;

                for (auto &a : i.appenders)
                {
                    YAML::Node na;
                    if (a.type == 1)
                    {
                        na["type"] = "FileLogAppender";
                        na["file"] = a.file;
                    }
                    else if (a.type == 2)
                    {
                        na["type"] = "StdoutLogAppender";
                    }
                    na["level"] = LogLevel::ToString(a.level);
                    na["formatter"] = a.formatter;
                    n["appenders"].push_back(na);
                }
                node.push_back(n);
            }
            std::stringstream ss;
            ss << node;
            return ss.str();
        }
    };

    fatdog::ConfigVar<std::set<LogDefine>>::ptr g_log_defines =
        fatdog::Config::lookUp("logs", std::set<LogDefine>(), "logs config");

    struct LogIniter
    {
        LogIniter()
        {
            g_log_defines->addListener([](const std::set<LogDefine> &old_value,
                                          const std::set<LogDefine> &new_value) {
                FATDOG_LOG_INFO(FATDOG_LOG_ROOT()) << "on_logger_conf_changed";
                for (auto &i : new_value)
                {
                    auto it = old_value.find(i);
                    fatdog::Logger::ptr logger;
                    if (it == old_value.end())
                    {
                        //新增logger
                        logger = FATDOG_LOG_NAME(i.name);
                    }
                    else
                    {
                        if (!(i == *it))
                        {
                            //修改的logger
                            logger = FATDOG_LOG_NAME(i.name);
                        }
                    }
                    logger->setLevel(i.level);
                    if (!i.formatter.empty())
                    {
                        logger->setFormatter(i.formatter);
                    }

                    logger->clearAppenders();
                    for (auto &a : i.appenders)
                    {
                        fatdog::LogAppender::ptr ap;
                        if (a.type == 1)
                        {
                            ap.reset(new FileLogAppender(a.file));
                        }
                        else if (a.type == 2)
                        {
                            ap.reset(new StdoutLogAppender);
                        }
                        ap->setLevel(a.level);
                        if (!a.formatter.empty())
                        {
                            LogFormatter::ptr fmt(new LogFormatter(a.formatter));
                            if (!fmt->isError())
                            {
                                ap->setFormatter(fmt);
                            }
                            else
                            {
                                FATDOG_LOG_INFO(FATDOG_LOG_ROOT()) << "log.name=" << i.name << " appender type=" << a.type
                                                                   << " formatter=" << a.formatter << " is invalid" << std::endl;
                            }
                        }
                        logger->addAppender(ap);
                    }
                }

                for (auto &i : old_value)
                {
                    auto it = new_value.find(i);
                    if (it == new_value.end())
                    {
                        //删除logger
                        auto logger = FATDOG_LOG_NAME(i.name);
                        logger->setLevel((LogLevel::Level)0);
                        logger->clearAppenders();
                    }
                }
            });
        }
    };

    static LogIniter __log_init;

} // namespace fatdog
