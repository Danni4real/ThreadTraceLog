#include <thread>

#include "ThreadTraceLog.h"

void func_3 TRACE(int,i, bool,b, float,f, char,c, const std::string&,s) // Notice: remove first left brace !!!

    {
        log_scope("scope");

        log_debug("%s", "debug");
        log_info("%s", "info");
        log_warn("%s", "warn");
        log_error("%s", "error");
    }
}

void func_2(int i) {
    log_call_format("i=%d", i);

    func_3(3, false, 3.14, 'a', "bar");
}

void func_1(const std::string& s) {
    log_call(s);

    func_2(2);
}

int main() {
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
