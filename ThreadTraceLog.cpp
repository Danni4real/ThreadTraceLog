//
// Created by dan on 2026/1/9.
//

#include "ThreadTraceLog.h"

#include <mutex>
#include <cctype>
#include <vector>
#include <string>
#include <cstdio>
#include <thread>
#include <sstream>
#include <cstdarg>
#include <unistd.h>
#include <iostream>
#include <algorithm>
#include <functional>


#define MODULE_NAME "Test"

//#define USE_SPDLOG_AS_PRINTER // default use spd log as printer
#ifdef USE_SPDLOG_AS_PRINTER

#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>

#define LOG_FILE_PATH "/tmp/thread_trace.log"
#define LOG_FILE_MAX_SIZE 10000 // bytes
#define LOG_FILE_MAX_NUM 6

class SpdLogWrapper {
public:
    static SpdLogWrapper &getInstance() {
        static SpdLogWrapper spd_log;
        return spd_log;
    }

    SpdLogWrapper() {
        auto logger = spdlog::rotating_logger_mt(
            "thread_trace_logger",      // logger name
            LOG_FILE_PATH,              // filename
            LOG_FILE_MAX_SIZE,
            LOG_FILE_MAX_NUM);

        spdlog::set_default_logger(logger);
        spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%f] %v");
        spdlog::set_level(spdlog::level::debug);
        spdlog::flush_on(spdlog::level::debug);
    }

    void shutdown() {
        spdlog::shutdown();
    }

    void spdlog_info(const std::string &msg) {
        std::cout << msg << std::endl;
        spdlog::info(msg);
    }
};
#endif

// trim helpers used by split_args_str
std::string &ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    }));
    return s;
}

std::string &rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
    }).base(), s.end());
    return s;
}

std::string &trim_inplace(std::string &s) {
    return ltrim(rtrim(s));
}

std::vector<std::string> split_args_str(const std::string &str) {
    std::vector<std::string> result;
    if (str.empty()) {
        return result;
    }

    std::stringstream ss(str);
    std::string item;

    while (std::getline(ss, item, ',')) {
        result.push_back(item);
    }

    return result;
}

std::string printf_to_string(const char *format, ...) {
    if (format == nullptr || *format == '\0') {
        return "";
    }

    va_list args;
    va_start(args, format);

    // Use va_copy to safely determine required buffer size
    va_list args_copy;
    va_copy(args_copy, args);
    int length = vsnprintf(nullptr, 0, format, args_copy);
    va_end(args_copy);

    if (length < 0) {
        va_end(args);
        return "";
    }

    std::string result;
    result.resize(static_cast<size_t>(length) + 1);

    int written = vsnprintf(result.data(), result.size(), format, args);
    va_end(args);

    if (written < 0) {
        return "";
    }

    result.resize(static_cast<size_t>(written));
    return result;
}

ThreadName &ThreadName::getInstance() {
    thread_local ThreadName n;
    return n;
}

const std::string &ThreadName::get() {
    return m_name;
}

void ThreadName::set(const std::string &name) {
    m_name = name;
}

static int ColorIndex = -1;
static std::mutex ColorLock;
static constexpr char ResetColorCode[] = "\x1b[0m";
static constexpr const char *ColorList[] = {
    "\x1b[31m", // red
    "\x1b[32m", // green
    "\x1b[33m", // yellow
    "\x1b[34m", // blue
    "\x1b[35m", // magenta
    "\x1b[36m", // cyan
    "\x1b[91m", // bright red
    "\x1b[92m", // bright green
    "\x1b[93m", // bright yellow
    "\x1b[94m", // bright blue
};

class ThreadColor {
public:
    static ThreadColor &getInstance() {
        thread_local ThreadColor t;
        return t;
    }

    [[nodiscard]] const char *color_code() const {
        return ColorList[m_color_index];
    }

private:
    int m_color_index;

    ThreadColor() {
        std::scoped_lock l(ColorLock);

        if (ColorIndex + 1 == static_cast<int>(std::size(ColorList))) {
            ColorIndex = 0;
        } else {
            ColorIndex++;
        }

        m_color_index = ColorIndex;
    }
};

class ThreadDepth {
public:
    static ThreadDepth &getInstance() {
        thread_local ThreadDepth d;
        return d;
    }

    [[nodiscard]] unsigned int get() const {
        return m_depth;
    }

    void increase_depth() {
        m_depth++;
    }

    void decrease_depth() {
        if (m_depth > 0) {
            m_depth--;
        }
    }

private:
    unsigned int m_depth{0};
};

void ThreadTraceLog::log_scope_enter(
    const std::string &file,
    const unsigned int line,
    const std::string &scope_name,
    const bool scope_is_a_function,
    const std::string &arg) {
    auto current_depth = ThreadDepth::getInstance().get();

    std::string log_msg;
    log_msg.reserve(256);
    log_msg.append(gen_head(InfoLevel));

    const std::string indent((current_depth > 0 ? current_depth - 1 : 0), '-');
    log_msg.append(indent);
    log_msg.append("> ");

    if (scope_is_a_function) {
        log_msg.append(refine_function_name(scope_name));

        log_msg.append("(");
        log_msg.append(arg);
        log_msg.append(")");
    } else {
        log_msg.append(scope_name);
    }

    log_msg.append(gen_tail(file, line));

    LogPrinter::getInstance().print(log_msg);
}

void ThreadTraceLog::log_scope_exit(
    const std::string &scope_name,
    const bool scope_is_a_function) {
    auto current_depth = ThreadDepth::getInstance().get();

    // same as enter: allow logging regardless of depth (defensive)
    std::string log_msg;
    log_msg.reserve(256);
    log_msg.append(gen_head(InfoLevel));

    log_msg.append("<");
    const std::string indent((current_depth > 0 ? current_depth - 1 : 0), '-');
    log_msg.append(indent);
    log_msg.append(" ");

    if (scope_is_a_function) {
        log_msg.append(refine_function_name(scope_name));
        log_msg.append("()");
    } else {
        log_msg.append(scope_name);
    }

    log_msg.append(ResetColorCode); // reset color

    LogPrinter::getInstance().print(log_msg);
}

void ThreadTraceLog::log(const std::string &file,
                         const unsigned int line,
                         const char level,
                         const std::string &info) {
    LogPrinter::getInstance().print(
        gen_log(file, line, level, info));
}

std::string ThreadTraceLog::gen_log(const std::string &file,
                                    const unsigned int line,
                                    const char level,
                                    const std::string &msg) {
    std::string log_msg;
    log_msg.reserve(256);
    log_msg.append(gen_head(level));

    const std::string indent(ThreadDepth::getInstance().get(), ' ');
    log_msg.append(indent);
    log_msg.append("  ");
    log_msg.append(msg);
    log_msg.append(gen_tail(file, line));

    return log_msg;
}

std::string ThreadTraceLog::refine_function_name(const std::string &func_name) {
    std::string refined(func_name);
    if (refined.empty()) {
        return "[empty_func]";
    }

    size_t bracket_pos = refined.find('(');
    if (bracket_pos != std::string::npos) {
        refined = refined.substr(0, bracket_pos);
    }

    size_t last_space_pos = refined.find_last_of(' ');
    if (last_space_pos != std::string::npos) {
        refined = refined.substr(last_space_pos + 1);
    }

    trim_inplace(refined);

    if (refined.empty()) {
        return "[invalid_func]";
    }

    return refined;
}

std::string ThreadTraceLog::gen_head(const char level) {
    const auto color_code = ThreadColor::getInstance().color_code();
    if (color_code == nullptr || *color_code == '\0') {
        return "";
    }

    std::string head;
    head.reserve(128);
    head.append("[");
    switch (level) {
        case InfoLevel: head.append("\x1b[92m"/*green*/); break;
        case WarnLevel: head.append("\x1b[93m"/*yellow*/); break;
        case ErrorLevel: head.append("\x1b[91m"/*red*/); break;
        case DebugLevel: head.append("\x1b[94m"/*blue*/); break;
            default:;
    }
    head.append(1,level);
    head.append(ResetColorCode);
    head.append("]");

    head.append(color_code);

    head.append("[");
    head.append(MODULE_NAME);
    head.append("][");

    if (ThreadName::getInstance().get().empty()) {
        head.append(std::to_string(gettid()));
    } else {
        head.append(ThreadName::getInstance().get());
    }

    head.append("]");

    return head;
}

std::string ThreadTraceLog::gen_tail(const std::string &file,
                                     const unsigned int line) {
    std::string tail;
    tail.reserve(128);

    tail.append(" --");
    tail.append(file);
    tail.append(":");
    tail.append(std::to_string(line));
    tail.append(ResetColorCode);

    return tail;
}

ThreadDepthKeeper::ThreadDepthKeeper(const std::string &file,
                                     const unsigned int line,
                                     const std::string &scope_name,
                                     const bool scope_is_a_function,
                                     const std::vector<std::string> &arg_names,
                                     const std::vector<std::string> &arg_values) {
    if (arg_names.size() != arg_values.size()) {
        m_active = false;
        return;
    }

    ThreadDepth::getInstance().increase_depth();
    m_active = true;

    std::string format_arg;
    format_arg.reserve(256);

    for (size_t i = 0; i < arg_names.size(); ++i) {
        if (i) format_arg.append(", ");
        format_arg.append(arg_names[i]);
        format_arg.append("=");
        format_arg.append(arg_values[i]);
    }

    ThreadTraceLog::log_scope_enter(file, line, scope_name, scope_is_a_function, format_arg);

    m_scope_name = scope_name;
    m_scope_is_a_function = scope_is_a_function;
}

ThreadDepthKeeper::ThreadDepthKeeper(const std::string &file,
                                     const unsigned int line,
                                     const std::string &scope_name,
                                     const bool scope_is_a_function,
                                     const std::string &arg) {
    ThreadDepth::getInstance().increase_depth();
    m_active = true;

    ThreadTraceLog::log_scope_enter(
        file, line, scope_name, scope_is_a_function, arg);

    m_scope_name = scope_name;
    m_scope_is_a_function = scope_is_a_function;
}

ThreadDepthKeeper::~ThreadDepthKeeper() {
    if (!m_active) {
        return;
    }

    ThreadTraceLog::log_scope_exit(m_scope_name,
                                   m_scope_is_a_function);

    ThreadDepth::getInstance().decrease_depth();
}

LogPrinter &LogPrinter::getInstance() {
    static LogPrinter log_printer;
    return log_printer;
}

void LogPrinter::set_printer(const std::function<void(const std::string &)> &printer) {
    m_printer = printer;
}

void LogPrinter::print(const std::string &log) const {
    if (m_printer) {
        m_printer(log);
        return;
    }

#ifdef USE_SPDLOG_AS_PRINTER
    SpdLogWrapper::getInstance().spdlog_info(log);
#else
    // fallback to console output if spdlog not enabled
    std::cout << log << std::endl;
#endif
}