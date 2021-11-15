cmake_minimum_required(VERSION 3.12)

set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)

project(CGull)

set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_OSX_DEPLOYMENT_TARGET "12.0")

add_subdirectory(deps)

add_executable(cgull src/main.cpp src/window.cpp src/editor.cpp src/renderer.cpp src/shader.cpp src/buffer.cpp)

set_property(TARGET cgull PROPERTY CXX_STANDARD 20)

find_package(Freetype REQUIRED)
find_package(glfw3 3.3 REQUIRED)

target_include_directories(cgull PRIVATE ${FREETYPE_INCLUDE_DIRS})
target_link_libraries(cgull PRIVATE glfw ${FREETYPE_LIBRARIES} glad)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(cgull PRIVATE Threads::Threads)