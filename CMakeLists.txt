cmake_minimum_required(VERSION 3.3)
project(assignment_4)

find_package(Threads)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wextra -Wall -Wpointer-arith -Wformat -Wfloat-equal -Winit-self \
-Wcast-qual -Wwrite-strings -Wshadow -Wstrict-prototypes -Wmissing-prototypes -Wundef \
-Wunreachable-code -Wbad-function-cast -Wstrict-overflow=5 -Winline -Wundef -Wnested-externs \
-Wlogical-op -Wformat=2 -Wredundant-decls -m64")

set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS} -O0 -g -gdwarf-3")

set(SOURCE_FILES main.c)
add_executable(assignment_4 ${SOURCE_FILES} main.c)
target_link_libraries(assignment_4 ${CMAKE_THREAD_LIBS_INIT})