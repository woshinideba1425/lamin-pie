#ifndef __CDLOG_H__
#define __CDLOG_H__

#define DEBUG 1
#include <cstdio>
#ifdef __cplusplus
extern "C" {
#endif 

typedef enum{
  LOG_VERBOSE,
  LOG_DEBUG,
  LOG_INFO,
  LOG_WARN,
  LOG_ERROR,
  LOG_FATAL
}LogLevel;

void LogPrintf(int level,const char*tag,const char*func,int line,const char*format,...);
void LogDump  (int level,const char*tag,const char*func,int line,const char*label,const unsigned char*data,int len);
void LogSetModuleLevel(const char*module,int level);
void LogParseModule(const char*module);
void LogParseModules(int argc,const char*argv[]);
void LogShutdown();
#ifdef __cplusplus
}
#endif 

#ifdef __cplusplus
#include <string>
#include <sstream>
#include <iostream>
#include <cstdarg>
#include <algorithm>
#if __cplusplus >= 202002L
#include <source_location>
#include <ctime>
#include <iomanip>
#include <cstring>
#endif
namespace lalog{
#if __cplusplus >= 202002L

// C++20 FixedString模板
template <size_t N>
struct FixedString {
    char data[N] {};

    constexpr FixedString(const char (&str)[N]) {
        std::copy_n(str, N, data);
    }

    constexpr bool operator==(const FixedString&) const = default;
    constexpr bool operator<(const FixedString& rhs) const {
        for (size_t i = 0; i < N; ++i) {
            if (data[i] != rhs.data[i]) {
                return data[i] < rhs.data[i];
            }
        }
        return false;
    }

    constexpr const char* c_str() const {
        return data;
    }
    constexpr size_t size() const {
        return N;
    }
};

template <FixedString TAG>
class LogMessage {
public:
    LogMessage(const std::source_location& loc = std::source_location::current(),int level=LOG_DEBUG);
    virtual ~LogMessage(); // at destruction will flush the message
    std::ostringstream& messageStream() {return stream_;}
    void messageSave(const char* format, ...);
private:
    std::string fileName_;
    std::string funcName_;
    int line_;
    int level_message;
    int level_module;
    std::ostringstream stream_;
    std::string log_entry_;
    long timestamp_; //second part
    long timeusec_;  //usecond part

};

// Free helper functions implemented in .cpp to keep templates header-only
void EnsureLogInit();
void EnqueueLog(const std::string& entry);
int QueryModuleLevel(const std::string& fileName);
namespace detail {
// Extract file name from a path
inline std::string extractFileName(const char* filePath) {
    if (!filePath) return "???";
    const char* fileName = filePath;
    const char* lastSlash = strrchr(filePath, '/');
    if (lastSlash) {
        fileName = lastSlash + 1;
    } else {
        lastSlash = strrchr(filePath, '\\');
        if (lastSlash) fileName = lastSlash + 1;
    }
    return std::string(fileName);
}

// Parse a readable function name
inline std::string parseFunctionName(const char* funcName) {
    if (!funcName) return "???";
    std::string func(funcName);

    size_t parenStart = func.find('(');
    if (parenStart != std::string::npos) func = func.substr(0, parenStart);

    auto lpos = func.find_first_not_of(' ');
    auto rpos = func.find_last_not_of(' ');
    if (lpos == std::string::npos) return "???";
    func = func.substr(lpos, rpos - lpos + 1);

    size_t lastSpace = func.rfind(' ');
    if (lastSpace != std::string::npos) func = func.substr(lastSpace + 1);
    size_t lastColon = func.rfind("::");
    if (lastColon != std::string::npos) func = func.substr(lastColon + 2);
    size_t lt = func.find('<');
    if (lt != std::string::npos) func = func.substr(0, lt);

    return func.empty() ? "???" : func;
}
} // namespace detail

// Template definitions must be in the header
template<FixedString TAG>
LogMessage<TAG>::LogMessage(const std::source_location& loc, int level)
    : level_message(level), level_module(level) {
    fileName_ = detail::extractFileName(loc.file_name());
    funcName_ = detail::parseFunctionName(loc.function_name());
    line_ = static_cast<int>(loc.line());
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    timestamp_ = ts.tv_sec;
    timeusec_ = ts.tv_nsec / 1000;
    EnsureLogInit();
    level_module = QueryModuleLevel(fileName_);
}

template<FixedString TAG>
LogMessage<TAG>::~LogMessage() {
    std::ostringstream oss;
    static const char* colors[] = {"\033[0m","\033[1m","\033[0;32m","\033[0;36m","\033[1;31m","\033[0;31m"};
    if (level_message >= level_module) {
        oss << std::setw(10) << std::setfill('0') << (timestamp_) << "." << std::setw(6) << (timeusec_);
        oss << " \033[0;32m[" << fileName_ << "]\033[0;34m ";
        oss << funcName_ << ":" << line_ << " " << colors[level_message];
        const std::string str(stream_.str());
        if (!str.empty()) oss << str;
        log_entry_ += oss.str();
        log_entry_ += "\033[0m\n";
        EnqueueLog(log_entry_);
    }
}

template<FixedString TAG>
void LogMessage<TAG>::messageSave(const char* format, ...) {
    static const std::string kTruncatedWarningText = "[...truncated...]";
    if (level_message >= level_module) {
        va_list arglist;
        char message[2048] = {0};
        va_start(arglist, format);
        const int nbrcharacters = vsnprintf(message, sizeof(message), format, arglist);
        va_end(arglist);
        if (nbrcharacters <= 0) {
            stream_ << '"' << format << '"';
        } else if (nbrcharacters >= static_cast<int>(sizeof(message))) {
            stream_ << message << kTruncatedWarningText;
        } else {
            stream_ << message;
        }
    }
}

#else
class LogMessage {
protected:
    const std::string file_;
    const std::string function_;
    const int line_;
    int level_message;
    int level_module;
    std::ostringstream stream_;
    std::string log_entry_;
    long timestamp_; //second part
    long timeusec_;  //usecond part
public:
    LogMessage(const std::string& file, const int line, const std::string& function,int level);
    virtual ~LogMessage(); // at destruction will flush the message
    std::ostringstream& messageStream() {return stream_;}
    void messageSave(const char* format, ...);
};
class FatalMessage:public LogMessage{
protected:
    int signal_;
public:
    FatalMessage(const std::string& file, const int line, const std::string& function,int signal);
    virtual ~FatalMessage();
};
#endif
}//namespace lalog

#if __cplusplus >= 202002L
#define LOG_MAKE_FS(str) []{ constexpr lalog::FixedString<sizeof(str)> s(str); return s; }()
#define LOG(level)        lalog::LogMessage<LOG_MAKE_FS(#level)>(std::source_location::current(), LOG_##level).messageStream()
#define LOG_IF(level,exp) if (exp) lalog::LogMessage<LOG_MAKE_FS(#level)>(std::source_location::current(), LOG_##level).messageStream()
#else
#define LOG(level)        lalog::LogMessage(__FILE__,__LINE__,__FUNCTION__,LOG_##level).messageStream()
#define LOG_IF(level,exp) if(exp)lalog::LogMessage(__FILE__,__LINE__,__FUNCTION__,LOG_##level).messageStream()
#endif /*endof __cplusplus*/
#endif

#ifdef __cplusplus
  #if __cplusplus >= 202002L
    #define LOG_PRINTF(level,...) lalog::LogMessage<LOG_MAKE_FS(__FILE__)>(std::source_location::current(), level).messageSave(__VA_ARGS__)
  #else
    #define LOG_PRINTF(level,...) lalog::LogMessage(__FILE__,__LINE__,__FUNCTION__,level).messageSave(__VA_ARGS__)
  #endif
#else
  #define LOG_PRINTF(level,...) LogPrintf(level,__FILE__,__FUNCTION__,__LINE__,__VA_ARGS__)
#endif

#ifndef DEBUG
    #define LOGV(...)
    #define LOGV_IF(x,...)
    #define LOGD(...)
    #define LOGD_IF(x,...)
    #define LOG_DUMP(tag,data,len)
    #define DUMPV(tag,data,len)
    #define DUMPV_IF(x,tag,data,len)
    #define DUMPD(tag,data,len)
    #define DUMPD_IF(x,tag,data,len)
#else
    #define LOGV(...)  LOG_PRINTF(LOG_VERBOSE,__VA_ARGS__) 
    #define LOGD(...)  LOG_PRINTF(LOG_DEBUG,__VA_ARGS__)
    #define LOGV_IF(x,...)  {if(x) LOG_PRINTF(LOG_VERBOSE,__VA_ARGS__);}
    #define LOGD_IF(x,...)  {if(x) LOG_PRINTF(LOG_DEBUG,__VA_ARGS__);}
    #define LOG_DUMP(tag,data,len) LogDump(LOG_DEBUG,__FILE__,__FUNCTION__,__LINE__,tag,data,len)
    #define DUMPV(tag,data,len) LogDump(LOG_VERBOSE,__FILE__,__FUNCTION__,__LINE__,tag,data,len)
    #define DUMPD(tag,data,len) LogDump(LOG_DEBUG,__FILE__,__FUNCTION__,__LINE__,tag,data,len)
    #define DUMPV_IF(x,tag,data,len) {if(x) LogDump(LOG_VERBOSE,__FILE__,__FUNCTION__,__LINE__,tag,data,len);}
    #define DUMPD_IF(x,tag,data,len) {if(x) LogDump(LOG_DEBUG,__FILE__,__FUNCTION__,__LINE__,tag,data,len);}
#endif

#define LOGI(...)  LOG_PRINTF(LOG_INFO,__VA_ARGS__) 
#define LOGW(...)  LOG_PRINTF(LOG_WARN,__VA_ARGS__) 
#define LOGE(...)  LOG_PRINTF(LOG_ERROR,__VA_ARGS__)
#define DUMPI(tag,data,len) LogDump(LOG_INFO,__FILE__,__FUNCTION__,__LINE__,tag,data,len)
#define DUMPW(tag,data,len) LogDump(LOG_WARN,__FILE__,__FUNCTION__,__LINE__,tag,data,len)
#define DUMPE(tag,data,len) LogDump(LOG_ERROR,__FILE__,__FUNCTION__,__LINE__,tag,data,len)
#if __cplusplus >= 202002L
  #define FATAL(...) lalog::LogMessage<LOG_MAKE_FS("FATAL")>(std::source_location::current(), LOG_FATAL).messageSave(__VA_ARGS__)
#else
  #define FATAL(...) lalog::FatalMessage(__FILE__,__LINE__,__FUNCTION__,0).messageSave(__VA_ARGS__)
#endif

#define LOGI_IF(x,...)  {if(x) LOG_PRINTF(LOG_INFO,__VA_ARGS__);}
#define LOGW_IF(x,...)  {if(x) LOG_PRINTF(LOG_WARN,__VA_ARGS__);}
#define LOGE_IF(x,...)  {if(x) LOG_PRINTF(LOG_ERROR,__VA_ARGS__);}
#define DUMPI_IF(x,tag,data,len) {if(x) LogDump(LOG_INFO,__FILE__,__FUNCTION__,__LINE__,tag,data,len);}
#define DUMPW_IF(x,tag,data,len) {if(x) LogDump(LOG_WARN,__FILE__,__FUNCTION__,__LINE__,tag,data,len);}
#define DUMPE_IF(x,tag,data,len) {if(x) LogDump(LOG_ERROR,__FILE__,__FUNCTION__,__LINE__,tag,data,len);}
#if __cplusplus >= 202002L
  #define FATAL_IF(x,...) {if(x) lalog::LogMessage<LOG_MAKE_FS("FATAL")>(std::source_location::current(), LOG_FATAL).messageSave(__VA_ARGS__);}
#else
  #define FATAL_IF(x,...) {if(x) lalog::FatalMessage(__FILE__,__LINE__,__FUNCTION__,0).messageSave(__VA_ARGS__);}
#endif
    
#endif//endif __CDLOG_H__
