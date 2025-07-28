cmake_minimum_required(VERSION 3.27)
project(MusicSuggestionsChecker)

set(CMAKE_CXX_STANDARD 14)

include_directories(src)

add_executable(MusicSuggestionsChecker
        src/main.cpp
        src/main.h)
