#include <thread>

#include "ThreadTraceLog.h"

void func_3 TRACE(int,i, bool,b, float,f, char,c, const std::string&,s) // Notice: remove first left brace !!!

    log_debug_var(i);
    log_info_var(b);
    log_warn_var(f);
    log_error_var(c);

    log_debug("i = ", i);
    log_info("b = ", b);
    log_warn("f = ", f);
    log_error("c = ", c);

    {
        log_scope("scope");

        log_debug_fmt("%s", "debug");
        log_info_fmt("%s", "info");
        log_warn_fmt("%s", "warn");
        log_error_fmt("%s", "error");
    }

    log_disable_info_print();
    log_disable_warn_print();
    log_disable_debug_print();

    log_debug("disabled ", "debug");
    log_info("disabled ", "info");
    log_warn("disabled ", "warn");
    log_error("not disabled ", "error");
}

void func_2(int i) {
    log_call_fmt("i=%d", i);

    func_3(3, false, 3.14, 'a', "bar");
}

void func_1(const std::string& s) {
    log_call(s);

    func_2(2);
}

int main() {
    log_set_process_name("test");
    log_call();

    std::thread t1([] {
        log_set_thread_name("thread_1");
        func_1("fool");
    });

    std::thread t2([] {
        log_set_thread_name("thread_2");
        func_1("fool");
    });

    t1.join();
    t2.join();

    return 0;
}
