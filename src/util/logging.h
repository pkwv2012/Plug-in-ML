// Copyright 2018 The RPSCC Authors. All Rights Reserved.
//
// Created by PeikaiZheng on 2018/12/8.
//

#ifndef SRC_UTIL_LOGGING_H_
#define SRC_UTIL_LOGGING_H_
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <mutex>
#include <string>
#include <vector>
#include <stdexcept>

namespace rpscc {

// exception class that will be thrown by
// default logger if RPSCC_LOG_FATAL_THROW == 1

struct Error : public std::runtime_error {
  // param s the error message
  explicit Error(const std::string &s) : std::runtime_error(s) {}
};
}  // namespace rpscc

#if defined(_MSC_VER) && _MSC_VER < 1900
#define noexcept(a)
#endif

#if RPSCC_USE_CXX11
#define RPSCC_THROW_EXCEPTION noexcept(false)
#else
#define RPSCC_THROW_EXCEPTION
#endif

#if RPSCC_USE_GLOG
#include <glog/logging.h>

namespace rpscc {
inline void InitLogging(const char* argv0) {
  google::InitGoogleLogging(argv0);
}
}  // namespace rpscc

#else
// use a light version of glog
#include <assert.h>
#include <iostream>
#include <sstream>
#include <ctime>

#if defined(_MSC_VER)
#pragma warning(disable : 4722)
#endif

namespace rpscc {
inline void InitLogging(const char* argv0) {
  // DO NOTHING
}

// Always-on checking
#define CHECK(x)                                           \
  if (!(x))                                                \
    rpscc::LogMessageFatal(__FILE__, __LINE__).stream() << "Check "  \
      "failed: " #x << ' '
#define CHECK_LT(x, y) CHECK((x) < (y))
#define CHECK_GT(x, y) CHECK((x) > (y))
#define CHECK_LE(x, y) CHECK((x) <= (y))
#define CHECK_GE(x, y) CHECK((x) >= (y))
#define CHECK_EQ(x, y) CHECK((x) == (y))
#define CHECK_NE(x, y) CHECK((x) != (y))
#define CHECK_NOTNULL(x) \
  ((x) == NULL ? rpscc::LogMessageFatal(__FILE__, __LINE__).stream() << "Check  notnull: "  #x << ' ', (x) : (x)) // NOLINT(*)
// Debug-only checking.
#ifdef NDEBUG
#define DCHECK(x) \
  while (false) CHECK(x)
#define DCHECK_LT(x, y) \
  while (false) CHECK((x) < (y))
#define DCHECK_GT(x, y) \
  while (false) CHECK((x) > (y))
#define DCHECK_LE(x, y) \
  while (false) CHECK((x) <= (y))
#define DCHECK_GE(x, y) \
  while (false) CHECK((x) >= (y))
#define DCHECK_EQ(x, y) \
  while (false) CHECK((x) == (y))
#define DCHECK_NE(x, y) \
  while (false) CHECK((x) != (y))
#else
#define DCHECK(x) CHECK(x)
#define DCHECK_LT(x, y) CHECK((x) < (y))
#define DCHECK_GT(x, y) CHECK((x) > (y))
#define DCHECK_LE(x, y) CHECK((x) <= (y))
#define DCHECK_GE(x, y) CHECK((x) >= (y))
#define DCHECK_EQ(x, y) CHECK((x) == (y))
#define DCHECK_NE(x, y) CHECK((x) != (y))
#endif  // NDEBUG

#define LOG_INFO rpscc::LogMessage(__FILE__, __LINE__, "INFO")
#define LOG_ERROR rpscc::LogMessage(__FILE__, __LINE__, "ERROR")
#define LOG_WARNING LOG_INFO
#define LOG_FATAL rpscc::LogMessageFatal(__FILE__, __LINE__)
#define LOG_QFATAL LOG_FATAL

// Poor man version of VLOG
#define VLOG(x) LOG_INFO.stream()

#define LOG(severity) LOG_##severity.stream()
#define LG LOG_INFO.stream()
#define LOG_IF(severity, condition) \
  !(condition) ? (void)0 : rpscc::LogMessageVoidify() & LOG(severity)

#ifdef NDEBUG
#define LOG_DFATAL LOG_ERROR
#define DFATAL ERROR
#define DLOG(severity) true ? (void)0 : rpscc::LogMessageVoidify() & LOG(severity)
#define DLOG_IF(severity, condition) \
  (true || !(condition)) ? (void)0 : rpscc::LogMessageVoidify() & LOG(severity)
#else
#define LOG_DFATAL LOG_FATAL
#define DFATAL FATAL
#define DLOG(severity) LOG(severity)
#define DLOG_IF(severity, condition) LOG_IF(severity, condition)
#endif

// Poor man version of LOG_EVERY_N
#define LOG_EVERY_N(severity, n) LOG(severity)

class DateLogger {
 public:
  DateLogger() {
#if defined(_MSC_VER)
    _tzset();
#endif
  }
  const char* HumanDate() {
#if defined(_MSC_VER)
    _strtime_s(buffer_, sizeof(buffer_));
#else
    time_t time_value = time(NULL);
    struct tm now;
    localtime_r(&time_value, &now);
    snprintf(buffer_, sizeof(buffer_), "%02d:%02d:%02d", now.tm_hour,
             now.tm_min, now.tm_sec);
#endif
    return buffer_;
  }
 private:
  char buffer_[9];
};

// The std::cerr and std::cout is thread-safe object, which means
// a single operator<< call is thread-safe, but when we call
// operator<< multiple times in one line it's not thread-safe, and
// the output will be overlapped.
// Having already try to use mutex to protect the std::cerr, but
// after the compiler's optimization, there is starvation.
// TODO: what will happen is the program crash before the destructor called?
// TODO: how to make the logging process-safe?
class LogMessage {
 public:
  LogMessage(const char* file, int line, const char* severity)
    :
#ifdef __ANDROID__
    log_stream_(std::cout)
#else
    log_stream_(std::cerr)
#endif
  {
    buffer_stream_ << "[" << pretty_date_.HumanDate() << "] "
                   << "(" << severity << ") " << file << ":"
                   << line << ": ";
  }
  ~LogMessage() {
    buffer_stream_ << '\n';
    log_stream_ << buffer_stream_.str();
  }
  std::stringstream& stream() { return buffer_stream_; }

 protected:
  std::ostream& log_stream_;
  std::stringstream buffer_stream_;

 private:
  DateLogger pretty_date_;
  LogMessage(const LogMessage&);
  void operator=(const LogMessage&);
};

#if RPSCC_LOG_FATAL_THROW == 0
class LogMessageFatal : public LogMessage {
 public:
  LogMessageFatal(const char* file, int line) : LogMessage(file, line, "FATAL") {}
  ~LogMessageFatal() {
    log_stream_ << "\n";
    abort();
  }

 private:
  LogMessageFatal(const LogMessageFatal&);
  void operator=(const LogMessageFatal&);
};
#else
class LogMessageFatal {
 public:
  LogMessageFatal(const char* file, int line) {
    log_stream_ << "[" << pretty_date_.HumanDate() << "] " << file << ":"
                << line << ": ";
  }
  std::ostringstream &stream() { return log_stream_; }
  ~LogMessageFatal() RPSCC_THROW_EXCEPTION {
    // throwing out of destructor is evil
    // hopefully we can do it here
    // also log the message before throw
    LOG(ERROR) << log_stream_.str();
    throw Error(log_stream_.str());
  }

 private:
  std::ostringstream log_stream_;
  DateLogger pretty_date_;
  LogMessageFatal(const LogMessageFatal&);
  void operator=(const LogMessageFatal&);
};
#endif

// This class is used to explicitly ignore values in the conditional
// logging macros.  This avoids compiler warnings like "value computed
// is not used" and "statement has no effect".
class LogMessageVoidify {
 public:
  LogMessageVoidify() {}
  // This has to be an operator with a precedence lower than << but
  // higher than "?:". See its usage.
  void operator&(std::ostream&) {}
};

}  // namespace rpscc

#endif  // RPSCC_USE_GLOG

#endif  // SRC_UTIL_LOGGING_H_
