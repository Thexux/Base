#ifndef LOG_H
#define LOG_H

#include <sstream>
#include <string>
#include <atomic>
#include <thread>

#define LOG_THREADID_WIDTH 7
#define LOG_FILENAME_WIDTH 20

enum class LogLevel {
    DEBUG,
    INFO,
    WARN,
    ERROR
};

enum Level {
    debugLevel = 0,
    infoLevel  = 1,
    warnLevel  = 2,
    errorLevel = 3
};

class AsyncLogger;

class LogStream : public std::ostringstream
{
public:
    LogStream(AsyncLogger &logger, LogLevel level, const char *file, int line)
        : _logger(logger), _level(level), _file(file), _line(line) {}
    LogStream(AsyncLogger &logger, LogLevel level, const char *file, int line, const char *fmt, ...);
    ~LogStream();

private:
    AsyncLogger &_logger;
    LogLevel _level;
    const char *_file;
    int _line;

    LogStream(const LogStream &);
    void operator=(const LogStream &);
};

#define LOG(level) LogStream(AsyncLogger::getInstance(), level, __FILE__, __LINE__)
#define LOG_DEBUG LOG(LogLevel::DEBUG)
#define LOG_INFO LOG(LogLevel::INFO)
#define LOG_WARN LOG(LogLevel::WARN)
#define LOG_ERROR LOG(LogLevel::ERROR)

#define TRACED(...) LogStream(AsyncLogger::getInstance(), LogLevel::DEBUG, __FILE__, __LINE__, __VA_ARGS__)
#define TRACEI(...) LogStream(AsyncLogger::getInstance(), LogLevel::INFO, __FILE__, __LINE__, __VA_ARGS__)
#define TRACEW(...) LogStream(AsyncLogger::getInstance(), LogLevel::WARN, __FILE__, __LINE__, __VA_ARGS__)
#define TRACEE(...) LogStream(AsyncLogger::getInstance(), LogLevel::ERROR, __FILE__, __LINE__, __VA_ARGS__)

class AsyncLogger
{
    friend class LogStream;

public:
    static AsyncLogger &getInstance();
    LogLevel getLevel();

private:
    AsyncLogger();
    ~AsyncLogger();
    void init();
    void pushLog(LogLevel level, const char* file, int line, const std::string &message);
    std::string logMessage(LogLevel level, const std::string &preamble, const std::string &message);
    std::string formatMessage(LogLevel level, const std::string &preamble, const std::string &message);
    std::string formatPreamble(LogLevel level, const char *file, int line, const std::thread::id &tid);

private:
    std::atomic<LogLevel> _currentLevel;

    AsyncLogger(const AsyncLogger &);
    void operator=(const AsyncLogger &);
};

#endif // LOG_H
