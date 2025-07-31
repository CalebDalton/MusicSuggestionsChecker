cmake_minimum_required(VERSION 3.16)
project(MusicSuggestionsChecker)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(CURL REQUIRED)

add_executable(MusicSuggestionsChecker
        src/main.cpp
)

# Link against CURL and stdc++fs (needed for filesystem in GCC 9)
target_link_libraries(MusicSuggestionsChecker PRIVATE CURL::libcurl stdc++fs)
