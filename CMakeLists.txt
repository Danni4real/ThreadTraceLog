CMAKE_MINIMUM_REQUIRED(VERSION 3.5)
PROJECT(ThreadTraceLog)

SET(Bin ThreadTraceLog)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Os -W -Wno-error=format-security --std=c++17")

INCLUDE_DIRECTORIES(
        /usr/include/
        ${CMAKE_CURRENT_SOURCE_DIR}/
)

LINK_DIRECTORIES(
        /lib
        /usr/lib
)

AUX_SOURCE_DIRECTORY(. SRC)
ADD_EXECUTABLE(${Bin} ${SRC})

TARGET_LINK_LIBRARIES(${Bin} pthread fmt spdlog)
