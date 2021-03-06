cmake_minimum_required(VERSION 2.8)
project(assignment_4 C)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif(NOT CMAKE_BUILD_TYPE)
message(STATUS "building for ${CMAKE_BUILD_TYPE}")

include(CheckCCompilerFlag)
find_package(Threads REQUIRED)

if (CMAKE_VERSION VERSION_LESS "3.1")
    if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
        set (CMAKE_C_FLAGS "--std=gnu99 ${CMAKE_C_FLAGS}")
    endif ()
else ()
    set (CMAKE_C_STANDARD 99)
endif ()

set(MY_C_FLAGS_LIST -Wextra -Wall -Wpointer-arith -Wformat -Wfloat-equal -Winit-self -Wcast-qual -Wwrite-strings -Wshadow -Wstrict-prototypes -Wmissing-prototypes -Wundef -Wunreachable-code -Wbad-function-cast -Wstrict-overflow=5 -Winline -Wundef -Wnested-externs -Wlogical-op -Wformat=2 -Wredundant-decls)
foreach(FLAG ${MY_C_FLAGS_LIST})
    check_c_compiler_flag(${FLAG} HAS_${FLAG})
    if (HAS_${FLAG})
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${FLAG}")
    endif()
endforeach()

if(UNIX)
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m64")
    else()
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m32")
    endif()
endif()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} $ENV{EXTRA_FLAGS}")

set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS} -O0 -g -gdwarf-3 -DDEBUG")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS} -O3")

set(SOURCE_FILES main.c io.c utils.h)
add_executable(assignment_4 ${SOURCE_FILES})
target_link_libraries(assignment_4 ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(assignment_4 m)