# ThreadTraceLog

A log message generator, features:

1. different color for different thread's log (up to 10 colors);
2. log_call(...) can print function name(auto) and it's arguments(user set);
3. auto print exit log when exit scope/function (if called log_call(...)/log_scope(...) before), which contains function/scope name;
4. log_call(...) and log_scope(...) increases log indent when enter scope/function, decreases log indent when exit scope/function, which simulates a code view;
5. use spdlog as default log printer, but can custom as you wish;
