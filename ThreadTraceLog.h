//
// Created by dan on 2026/1/9.
//

#ifndef THREADTRACELOG_H
#define THREADTRACELOG_H

#include <tuple>
#include <vector>
#include <string>
#include <cstring>
#include <sstream>
#include <utility>
#include <functional>

// pass all or some call arguments
#define log_call(...) \
    ThreadDepthKeeper k1(FILENAME_, __LINE__, __PRETTY_FUNCTION__, true, split_args_str(std::string(#__VA_ARGS__)), ArgValues(__VA_ARGS__).str_list());

// require format arguments
#define log_call_format(...) \
    ThreadDepthKeeper k1(FILENAME_, __LINE__, __PRETTY_FUNCTION__, true, printf_to_string(__VA_ARGS__));

// require const char* argument
#define log_scope(scope_name) ThreadDepthKeeper k2(FILENAME_, __LINE__, scope_name, false)

// log_info/log_warn/log_error/log_debug require format arguments
#define log_info(...)  ThreadTraceLog::log(FILENAME_, __LINE__, InfoLevel, printf_to_string(__VA_ARGS__))
#define log_warn(...)  ThreadTraceLog::log(FILENAME_, __LINE__, WarnLevel, printf_to_string(__VA_ARGS__))
#define log_error(...) ThreadTraceLog::log(FILENAME_, __LINE__, ErrorLevel, printf_to_string(__VA_ARGS__))
#define log_debug(...) ThreadTraceLog::log(FILENAME_, __LINE__, DebugLevel, printf_to_string(__VA_ARGS__))

// require const char* argument
#define log_set_thread_name(...) ThreadName::getInstance().set(__VA_ARGS__)

#define FILENAME_ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

constexpr char InfoLevel = 'I';
constexpr char WarnLevel = 'W';
constexpr char ErrorLevel = 'E';
constexpr char DebugLevel = 'D';

std::string printf_to_string(const char *format, ...);

std::vector<std::string> split_args_str(const std::string &str);

template<typename T>
std::string to_str(const T &value) {
    std::ostringstream oss;
    oss << value;
    return oss.str();
}

// overload for std::string to avoid extra streaming overhead
template<>
inline std::string to_str(const std::string &value) {
    return value;
}

// overload for C strings: produce their contents instead of pointer formatting
inline std::string to_str(const char *value) {
    return value ? std::string(value) : std::string("");
}

class LogPrinter {
public:
    static LogPrinter &getInstance();

    void set_printer(const std::function<void(const std::string &)> &printer);

    void print(const std::string &log) const;

private:
    std::function<void(const std::string &)> m_printer;
};


template<typename... Args>
class ArgValues {
    std::tuple<Args...> arg_values;
    std::vector<std::string> arg_values_strs;

public:
    explicit ArgValues(Args... values) : arg_values(std::forward<Args>(values)...) {
        std::apply([this](const auto &... elems) {
            (arg_values_strs.push_back(to_str(elems)), ...);
        }, arg_values);
    }

    [[nodiscard]] std::vector<std::string> str_list() const {
        return arg_values_strs;
    }
};


class ThreadTraceLog {
public:
    static void log_scope_enter(const std::string &file,
                                unsigned int line,
                                const std::string &scope_name,
                                bool scope_is_a_function,
                                const std::string &arg = {});

    static void log_scope_exit(const std::string &scope_name,
                               bool scope_is_a_function);

    static void log(const std::string &file,
                    unsigned int line,
                    char level,
                    const std::string &info);

private:
    static std::string gen_log(const std::string &file,
                               unsigned int line,
                               char level,
                               const std::string &msg);

    static std::string refine_function_name(const std::string &func_name);

    static std::string gen_head(char level);

    static std::string gen_tail(const std::string &file,
                                unsigned int line);
};


class ThreadDepthKeeper {
public:
    ThreadDepthKeeper(const std::string &file,
                      unsigned int line,
                      const std::string &scope_name,
                      bool scope_is_a_function,
                      const std::vector<std::string> &arg_names = {},
                      const std::vector<std::string> &arg_values = {});

    ThreadDepthKeeper(const std::string &file,
                      unsigned int line,
                      const std::string &scope_name,
                      bool scope_is_a_function,
                      const std::string &arg);

    ~ThreadDepthKeeper();

private:
    std::string m_scope_name;
    bool m_scope_is_a_function{false};
    bool m_active{false};
};

class ThreadName {
public:
    static ThreadName &getInstance();

    const std::string &get();

    void set(const std::string &name);

private:
    std::string m_name;
};

#endif //THREADTRACELOG_H