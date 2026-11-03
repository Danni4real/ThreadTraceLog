# ThreadTraceLog

A C++ logger, features:

1. Can set process and thread name;
2. Different color for different thread;
3. Print function name and it's arguments;
4. Print log when enter/exit scope/function;
5. Indent/Unindent when enter/exit scope/function;
6. Integrate spdlog as log printer (can switch off);
7. Integrate magic_enum to print enum name (can switch off);

Note:

  Must add -std=gnu++17 (or above) to compile flags
