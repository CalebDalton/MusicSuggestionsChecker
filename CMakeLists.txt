cmake_minimum_required(VERSION 3.27)
project(MusicSuggestionsChecker)

set(CMAKE_CXX_STANDARD 14)

# Find CURL via vcpkg
find_package(CURL REQUIRED)

# Include dirs
include_directories(src)

# Collect all sources for the spotify-api-plusplus library
file(GLOB_RECURSE SPOTIFY_SRC_H "spotify-api-plusplus/*.h")
file(GLOB_RECURSE SPOTIFY_SRC_CPP "spotify-api-plusplus/*.cpp")
set(SPOTIFY_SRC ${SPOTIFY_SRC_H} ${SPOTIFY_SRC_CPP})

# Create the shared library
add_library(spotify-api-plusplus SHARED ${SPOTIFY_SRC})

# Link CURL to the library
target_link_libraries(spotify-api-plusplus CURL::libcurl)

# Add the main executable
add_executable(MusicSuggestionsChecker
        src/main.cpp
)

# Link the library to the executable
target_link_libraries(MusicSuggestionsChecker spotify-api-plusplus)

# Install rules
install(TARGETS spotify-api-plusplus DESTINATION lib)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/src/ DESTINATION include/spotify-api-plusplus FILES_MATCHING PATTERN "*.h*")
