#pragma once

#include <cstddef>
#include <iostream>
#include <sstream>

namespace Starbase {
namespace Log {

enum Severity {
    trace,
    debug,
    info,
    notice,
    warning,
    error,
    critical,
    fatal
};

/*
 * Prevents unused variable warning, while keeping both enum and string
 * definitions together.
 */
#ifdef STARBASE_LOGGING_IMPL
static const char* SEVERITY_TEXT_REPR[] = {
    "TRACE",
    "DEBUG",
    "INFO",
    "NOTICE",
    "WARNING",
    "ERROR",
    "CRITICAL",
    "FATAL"
};
#endif

class SimpleLogStream {
private:
    Severity severity;
    std::string file;
    std::string func;
    int line;
    std::stringstream ss;

public:
    SimpleLogStream(Severity severity, const char* file, const char* func, int line);

    ~SimpleLogStream();

    bool IsWarningOrHigher() const;

    std::ostream& PhysicalStream() const;

    template<class T>
    SimpleLogStream& operator<<(const T &x)
    {
        ss << x;
        return *this;
    }
};

} // namespace Log
} // namespace Starbase

/*
 * Logs a message.
 * Usage: LOG(<severity>) << "<text to output>";
 */
#define LOG(severity) Starbase::Log::SimpleLogStream(Starbase::Log::severity, __FILE__, __func__, __LINE__)