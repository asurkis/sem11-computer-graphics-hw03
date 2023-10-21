cmake_minimum_required(VERSION 3.11)
project(computer_graphics_hw02)

set(CMAKE_CXX_STANDARD 17)

set(EXE_NAME hw02)
set(CXX_SOURCES
  main.cpp
  third-party/glad/src/gl.c
)

# find_package(OpenGL REQUIRED)

add_subdirectory(third-party/glfw)

add_executable(${EXE_NAME} ${CXX_SOURCES})
target_include_directories(${EXE_NAME} PRIVATE
  third-party/glfw/include
  third-party/glad/include
)
target_link_libraries(${EXE_NAME} PRIVATE glfw)
