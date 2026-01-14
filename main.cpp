#include <thread>
#include <unistd.h>

#include "ThreadTraceLog.h"

void func_3(int i, bool b, float f, char c, std::string s) {
    log_call(i,b,f,c,s);
    log_debug("debug message: %d",333);

    {
        log_scope("scope_a");
    }
}

void func_2(int x) {
    log_call_format("x=%d",x);
    log_info("222");

    func_3(100, false, 3.14, '(', "");
}

void func_1() {
    log_call();
    ThreadTraceLog::log_info("111");

    func_2(20);
}

void func_0() {
    log_call();
    log_debug("000+");

    func_1();
    log_warn("000-");
}

int main() {
    log_call();

    std::thread([] {
        log_set_thread_name("thread_1");
        func_0();
    }).detach();

    std::thread([] {
        log_set_thread_name("thread_2");
        func_0();
    }).detach();



    sleep(1);

    return 0;
}