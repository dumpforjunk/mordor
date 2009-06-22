#ifndef __LOG_H__
#define __LOG_H__
// Copyright (c) 2009 - Decho Corp.

#include <list>
#include <set>
#include <sstream>

#include <boost/enable_shared_from_this.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread/thread.hpp>

#include "version.h"

class Logger;
class LogSink;

#ifdef ERROR
#undef ERROR
#endif

class Log
{
private:
    Log();

public:
    enum Level {
        FATAL,
        ERROR,
        WARNING,
        INFO,
        TRACE,
        VERBOSE,
        DEBUG
    };

    static boost::shared_ptr<Logger> lookup(const std::string &name);

    static void addSink(boost::shared_ptr<LogSink> sink);
    static void clearSinks();

private:
    static Logger *m_root;
    static boost::shared_ptr<Logger> m_rootRef;
};

#ifdef WINDOWS
typedef DWORD tid_t;
#else
typedef pid_t tid_t;
#endif

class LogSink
{
    friend class Logger;
public:
    typedef boost::shared_ptr<LogSink> ptr;
public:

    virtual ~LogSink() {}
    virtual void log(const std::string &logger, tid_t thread, void *fiber,
        Log::Level level, const std::string &str,
        const char *file, int line) = 0;
};

class StdoutLogSink : public LogSink
{
public:
    void log(const std::string &logger, tid_t thread, void *fiber,
        Log::Level level, const std::string &str,
        const char *file, int line);
};

struct LogStream : public std::ostringstream
{
    friend class Logger;
private:
    LogStream(boost::shared_ptr<Logger> logger, Log::Level level,
        const char *file, int line)
        : m_logger(logger),
          m_level(level),
          m_file(file),
          m_line(line)
    {}

    LogStream(const LogStream &copy)
        : m_logger(copy.m_logger),
          m_level(copy.m_level),
          m_file(copy.m_file),
          m_line(copy.m_line)
    {}
public:
    ~LogStream();

private:
    boost::shared_ptr<Logger> m_logger;
    Log::Level m_level;
    const char *m_file;
    int m_line;
};

struct LoggerLess
{
    bool operator()(const boost::shared_ptr<Logger> &lhs,
        const boost::shared_ptr<Logger> &rhs) const;
};

class Logger : public boost::enable_shared_from_this<Logger>
{
    friend class Log;
    friend struct LoggerLess;
public:
    typedef boost::shared_ptr<Logger> ptr;
private:
    Logger();
    Logger(const std::string &name, Logger::ptr parent);

public:
    bool enabled(Log::Level level) { return m_level >= level; }
    void level(Log::Level level, bool propagate = true);
    Log::Level level() const { return m_level; }

    bool inheritSinks() const { return m_inheritSinks; }
    void inheritSinks(bool inherit) { m_inheritSinks = inherit; }
    void addSink(LogSink::ptr sink) { m_sinks.push_back(sink); }
    void clearSinks() { m_sinks.clear(); }

    LogStream log(Log::Level level, const char *file = NULL, int line = -1)
    { return LogStream(shared_from_this(), level, file, line); }
    void log(Log::Level level, const std::string &str, const char *file = NULL, int line = 0);

    LogStream debug(const char *file = NULL, int line = -1)
    { return log(Log::DEBUG, file, line); }
    LogStream verbose(const char *file = NULL, int line = -1)
    { return log(Log::VERBOSE, file, line); }
    LogStream trace(const char *file = NULL, int line = -1)
    { return log(Log::TRACE, file, line); }
    LogStream info(const char *file = NULL, int line = -1)
    { return log(Log::INFO, file, line); }
    LogStream warning(const char *file = NULL, int line = -1)
    { return log(Log::WARNING, file, line); }
    LogStream error(const char *file = NULL, int line = -1)
    { return log(Log::ERROR, file, line); }
    LogStream fatal(const char *file = NULL, int line = -1)
    { return log(Log::FATAL, file, line); }

    std::string name() const { return m_name; }

private:
    std::string m_name;
    Logger::ptr m_parent;
    std::set<Logger::ptr, LoggerLess> m_children;
    Log::Level m_level;
    std::list<LogSink::ptr> m_sinks;
    bool m_inheritSinks;
};

#define LOG_DEBUG(log) (log)->debug(__FILE__, __LINE__)
#define LOG_VERBOSE(log) (log)->verbose(__FILE__, __LINE__)
#define LOG_TRACE(log) (log)->trace(__FILE__, __LINE__)
#define LOG_INFO(log) (log)->info(__FILE__, __LINE__)
#define LOG_WARNING(log) (log)->warning(__FILE__, __LINE__)
#define LOG_ERROR(log) (log)->error(__FILE__, __LINE__)
#define LOG_FATAL(log) (log)->fatal(__FILE__, __LINE__)

std::ostream &operator <<(std::ostream &os, Log::Level level);

#endif
