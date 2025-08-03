cmake_minimum_required(VERSION 3.16)
project(MusicSuggestionsChecker)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


find_package(OpenGL REQUIRED)
find_package(glfw3 CONFIG REQUIRED)

add_executable(MusicSuggestionsChecker
        src/main.cpp
        ${CMAKE_SOURCE_DIR}/imgui/imgui.cpp
        ${CMAKE_SOURCE_DIR}/imgui/imgui_draw.cpp
        ${CMAKE_SOURCE_DIR}/imgui/imgui_tables.cpp
        ${CMAKE_SOURCE_DIR}/imgui/imgui_widgets.cpp
        ${CMAKE_SOURCE_DIR}/imgui/imgui_demo.cpp
        ${CMAKE_SOURCE_DIR}/imgui/backends/imgui_impl_glfw.cpp
        ${CMAKE_SOURCE_DIR}/imgui/backends/imgui_impl_opengl3.cpp
)

target_include_directories(MusicSuggestionsChecker PRIVATE
        ${CMAKE_SOURCE_DIR}/imgui
        ${CMAKE_SOURCE_DIR}/imgui/backends
        OpenGL::GL
        glfw
)

target_link_libraries(MusicSuggestionsChecker PRIVATE stdc++fs glfw)