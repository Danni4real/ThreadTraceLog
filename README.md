# ThreadTraceLog

A log message generator, features:

1. different color for different thread's log (up to 10 colors);
2. user can set thread name to increase log intuitively: 
3. log_call(...) can print function name(auto) and it's arguments(user set);
4. auto print exit log when exit scope/function (if called log_call(...)/log_scope(...) before), which contains function/scope name;
5. log_call(...) and log_scope(...) increases log indent when enter scope/function, decreases log indent when exit scope/function, which simulates a code view;
6. use spdlog as default log printer, but can custom as you wish;
