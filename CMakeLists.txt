cmake_minimum_required(VERSION 3.11)
project(CGull)

set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_subdirectory(deps)

add_executable(cgull src/main.cpp src/editor.cpp)

find_package(Freetype REQUIRED)
find_package(glfw3 3.3 REQUIRED)

target_include_directories(cgull PRIVATE ${FREETYPE_INCLUDE_DIRS})
target_link_libraries(cgull PRIVATE glfw ${FREETYPE_LIBRARIES} glad)
