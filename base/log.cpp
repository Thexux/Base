#include <iostream>
#include <format>
#include <chrono>
#include <cstdarg>
#include "log.h"

#ifdef _WIN32
	#define localtime_r(a, b) localtime_s(b, a)
#endif

bool g_terminalHasColor = true;

#ifdef _WIN32
#define VTSEQ(ID) ("\x1b[1;" #ID "m")
#else
#define VTSEQ(ID) ("\x1b[" #ID "m")
#endif

const char* terminalBlack()      { return g_terminalHasColor ? VTSEQ(30) : ""; }
const char* terminalRed()        { return g_terminalHasColor ? VTSEQ(31) : ""; }
const char* terminalGreen()      { return g_terminalHasColor ? VTSEQ(32) : ""; }
const char* terminalYellow()     { return g_terminalHasColor ? VTSEQ(33) : ""; }
const char* terminalBlue()       { return g_terminalHasColor ? VTSEQ(34) : ""; }
const char* terminalPurple()     { return g_terminalHasColor ? VTSEQ(35) : ""; }
const char* terminalCyan()       { return g_terminalHasColor ? VTSEQ(36) : ""; }
const char* terminalLightGray()  { return g_terminalHasColor ? VTSEQ(37) : ""; }
const char* terminalWhite()      { return g_terminalHasColor ? VTSEQ(37) : ""; }
const char* terminalLightRed()   { return g_terminalHasColor ? VTSEQ(91) : ""; }
const char* terminalDim()        { return g_terminalHasColor ? VTSEQ(2)  : ""; }

const char* terminalBold()       { return g_terminalHasColor ? VTSEQ(1) : ""; }
const char* terminalUnderline()  { return g_terminalHasColor ? VTSEQ(4) : ""; }

const char* terminalReset()      { return g_terminalHasColor ? VTSEQ(0) : ""; }


LogStream::LogStream(AsyncLogger &logger, LogLevel level, const char *file, int line, const char *fmt, ...)
    : _logger(logger), _level(level), _file(file), _line(line)
{
    va_list args;
    va_start(args, fmt);
    
    va_list argsCopy;
    va_copy(argsCopy, args);
    int len = vsnprintf(nullptr, 0, fmt, argsCopy) + 1;
    va_end(argsCopy);
   
    if (len > 0)
    {
        auto buf = std::make_unique<char[]>(len);
        vsnprintf(buf.get(), len, fmt, args);
        str(buf.get());
    }
    va_end(args);
}

LogStream::~LogStream()
{
    if (_level >= _logger.getLevel()) _logger.pushLog(_level, _file, _line, str());
}

AsyncLogger &AsyncLogger::getInstance()
{
    static AsyncLogger instance;
    return instance;
}

LogLevel AsyncLogger::getLevel()
{
    return _currentLevel;
}

AsyncLogger::AsyncLogger()
{
    init();
}

AsyncLogger::~AsyncLogger()
{
        
}

void AsyncLogger::init()
{
    _currentLevel = LogLevel::DEBUG;
}

void AsyncLogger::pushLog(LogLevel level, const char* file, int line, const std::string &message)
{
    std::string formatStr = logMessage(level, formatPreamble(level, file, line, std::this_thread::get_id()), message);

    std::cout << formatStr;
}

std::string AsyncLogger::logMessage(LogLevel level, const std::string &preamble, const std::string &message)
{
    std::string res;
    size_t start = 0;
    size_t end = message.find('\n');
    while (end != std::string::npos)
    {
        res += formatMessage(level, preamble, message.substr(start, end - start));
        start = end + 1;
        end = message.find('\n', start);
    }
    if (start < message.size()) res += formatMessage(level, preamble, message.substr(start));
    return res;
}

std::string AsyncLogger::formatMessage(LogLevel level, const std::string &preamble, const std::string &message)
{
    std::string res;
    if ((int)level < warnLevel)
    {
        res += terminalReset();
        res += terminalDim();
        res += preamble;
        res += (level == LogLevel::INFO ? terminalReset() : "");
        res += message;
        res += terminalReset();
        res += '\n';
    }
    else 
    {
        res += terminalReset();
        res += (level == LogLevel::ERROR ? terminalRed() : terminalYellow());
        res += preamble;
        res += message;
        res += terminalReset();
        res += '\n';
    }
    return res;
}

std::string AsyncLogger::formatPreamble(LogLevel level, const char *file, int line, const std::thread::id &tid)
{
    using namespace std::chrono;
    auto msSinceEpoch = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
    time_t secSinceEpoch = time_t(msSinceEpoch / 1000);
    tm timeInfo;
    localtime_r(&secSinceEpoch, &timeInfo);
    std::string res;
    res += std::format("[{:04d}-{:02d}-{:02d} {:02d}:{:02d}:{:02d}.{:03d}] ", 
                        timeInfo.tm_year + 1900, 
                        timeInfo.tm_mon + 1, 
                        timeInfo.tm_mday, 
                        timeInfo.tm_hour, 
                        timeInfo.tm_min, 
                        timeInfo.tm_sec, 
                        msSinceEpoch % 1000);
        
    res += std::format("[{:^{}}] ", tid, LOG_THREADID_WIDTH);
        
    const char* lastSlash = strrchr(file, '/');
    if (!lastSlash) lastSlash = strrchr(file, '\\');
    res += std::format("{:>{}}:{:<5d} ", lastSlash ? lastSlash + 1 : file, LOG_FILENAME_WIDTH, line);

    std::string levelStr;
    if (level == LogLevel::DEBUG) levelStr = "DEBUG";
    else if (level == LogLevel::INFO) levelStr = "INFO";
    else if (level == LogLevel::WARN) levelStr = "WARN";
    else levelStr = "ERROR";
            
    res += std::format("{:>5}| ", levelStr);
        
    return res;
}
