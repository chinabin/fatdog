#ifndef __FATDOG_LOG__
#define __FATDOG_LOG__

#include <string>
#include <memory>  // for shared_ptr
#include <cstdint> // for int8_t int16_t etc type
#include <vector>
#include <sstream>
#include <fstream>

/*
Logger: 负责产生日志信息，接收端
LogAppender: 负责将日志信息落地，例如文件、输出窗口，输出端
LogFormatter: 负责格式化日志信息，处于接收端到输出端的中间
*/

/*
Logger::log 调用 LogAppender::log ，后者调用 LogFormatter::format 返回格式化的字符串
*/
namespace fatdog
{

    class LogAppender;
    class LogFormatter;
    class Logger;

    class LogLevel
    {
    public:
        enum Level
        {
            UNKNOWN,
            DEBUG,
            INFO,
            WARN,
            ERROR,
            FATAL,
        };

        static std::string ToString(Level);
        static Level FromString(const std::string &str);
    };

    class LogEvent
    {
    public:
        typedef std::shared_ptr<LogEvent> ptr;

        LogEvent(std::shared_ptr<Logger> logger, LogLevel::Level level, const char *file, int32_t line, uint32_t elapse, uint32_t thread_id, uint32_t fiber_id, uint64_t time, const std::string &thread_name);

        const char *getFile() const { return m_filename; }
        uint32_t getLine() const { return m_line; }
        uint32_t getElapse() const { return m_elapse; }
        uint32_t getThreadId() const { return m_threadID; }
        uint32_t getFiberId() const { return m_fiberID; }
        uint64_t getTime() const { return m_time; }
        const std::string &getThreadName() const { return m_threadName; }
        std::string getContent() const { return m_ss.str(); }
        std::shared_ptr<Logger> getLogger() const { return m_logger; }
        LogLevel::Level getLevel() const { return m_level; }
        std::stringstream &getSS() { return m_ss; }

        void format(const char* fmt, ...);
    private:
        uint32_t m_line;
        uint32_t m_threadID;
        uint32_t m_fiberID;
        const char *m_filename = nullptr;
        // 程序启动开始到现在的毫秒数
        uint32_t m_elapse = 0;
        // 时间戳
        uint64_t m_time = 0;
        // 日志内容流
        std::stringstream m_ss;
        // 日志器
        std::shared_ptr<Logger> m_logger;
        // 日志等级
        LogLevel::Level m_level;
        std::string m_threadName;
    };

    class Logger : public std::enable_shared_from_this<Logger>
    {
    public:
        typedef std::shared_ptr<Logger> ptr;
        Logger(const std::string &name = "root", const LogLevel::Level level = LogLevel::INFO);
        ~Logger();

    public:
        void debug(LogEvent::ptr event);
        void info(LogEvent::ptr event);
        void warn(LogEvent::ptr event);
        void error(LogEvent::ptr event);
        void fatal(LogEvent::ptr event);

    private:
        void log(LogLevel::Level level, LogEvent::ptr event);

    public:
        std::string getName() const { return m_name; }

        LogLevel::Level getLevel() const { return m_level; }
        void setLevel(const LogLevel::Level level) { m_level = level; }

        void addAppender(std::shared_ptr<LogAppender> appender);
        void delAppender(std::shared_ptr<LogAppender> appender);
        void clearAppenders();

        void setFormatter(std::shared_ptr<LogFormatter> formatter);
        std::shared_ptr<LogFormatter> getFormatter() const { return m_formatter; }

    private:
        std::string m_name;
        LogLevel::Level m_level;

        std::vector<std::shared_ptr<LogAppender>> m_appenders;
        std::shared_ptr<LogFormatter> m_formatter; // sometimes, I just want it default
    };

    class LogAppender
    {
    public:
        typedef std::shared_ptr<LogAppender> ptr;

        LogAppender(LogLevel::Level level = LogLevel::INFO)
            : m_level(level)
        {
        }
        virtual ~LogAppender(){};

        void setFormatter(std::shared_ptr<LogFormatter> formatter) { m_formatter = formatter; }
        std::shared_ptr<LogFormatter> getFormatter() const { return m_formatter; }

        LogLevel::Level getLevel() const { return m_level; }
        void setLevel(const LogLevel::Level level) { m_level = level; }

        virtual void log(Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) = 0;

    protected:
        LogLevel::Level m_level = LogLevel::INFO;
        std::shared_ptr<LogFormatter> m_formatter = nullptr;
    };

    class StdoutLogAppender : public LogAppender
    {
    public:
        typedef std::shared_ptr<StdoutLogAppender> ptr;

        StdoutLogAppender(LogLevel::Level level = LogLevel::INFO)
            : LogAppender(level)
        {
        }

        virtual void log(Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override;
    };

    class FileLogAppender : public LogAppender
    {
    public:
        typedef std::shared_ptr<FileLogAppender> ptr;
        FileLogAppender(const std::string &filename, LogLevel::Level level = LogLevel::INFO);
        void log(Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override;

        bool reopen();

    private:
        /// 文件路径
        std::string m_filename;
        /// 文件流
        std::ofstream m_filestream;
        /// 上次重新打开时间
        uint64_t m_lastTime = 0;
    };

    class LogFormatter
    {
    public:
        typedef std::shared_ptr<LogFormatter> ptr;

        /*
        *  %m 消息
        *  %p 日志级别
        *  %r 累计毫秒数
        *  %c 日志名称
        *  %t 线程id
        *  %n 换行
        *  %d 时间
        *  %f 文件名
        *  %l 行号
        *  %T 制表符
        *  %F 协程id
        *  %N 线程名称
        *
        *  默认格式 "%d{%Y-%m-%d %H:%M:%S}%T%t%T%N%T%F%T[%p]%T[%c]%T%f:%l%T%m%n"
        */
        LogFormatter(const std::string &pattern = "%d{%Y-%m-%d %H:%M:%S}%T%t%T%N%T%F%T[%p]%T[%c]%T%f:%l%T%m%n");

        std::string format(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event);
        void init();

        bool isError() const { return m_error; }
        const std::string getPattern() const { return m_pattern; }

    public:
        class FormatItem
        {
        public:
            typedef std::shared_ptr<FormatItem> ptr;

            virtual ~FormatItem() {}

            virtual void format(std::ostream &os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) = 0;
        };

    private:
        std::string m_pattern;
        std::vector<FormatItem::ptr> m_items;
        bool m_error = false; // 解析的时候是否有错误
    };

} // namespace fatdog

#endif