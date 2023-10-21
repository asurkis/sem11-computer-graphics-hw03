cmake_minimum_required(VERSION 3.11)
project(computer_graphics_hw02)

set(CMAKE_CXX_STANDARD 17)

set(EXE_NAME hw02)
set(CXX_SOURCES
  main.cpp
  routine.hpp
  third-party/glad/src/gl.c
  third-party/imgui/imgui.cpp
  third-party/imgui/imgui_demo.cpp
  third-party/imgui/imgui_draw.cpp
  third-party/imgui/imgui_tables.cpp
  third-party/imgui/imgui_widgets.cpp
  third-party/imgui/imgui_impl_opengl3.cpp
  third-party/imgui/imgui_impl_glfw.cpp
  third-party/tinygltf/tiny_gltf.cc
)

# find_package(OpenGL REQUIRED)

add_subdirectory(third-party/glfw)
add_subdirectory(third-party/glm)

add_executable(${EXE_NAME} ${CXX_SOURCES})
target_include_directories(${EXE_NAME} PRIVATE
  third-party/glad/include
  third-party/glfw/include
  third-party/glm
  third-party/imgui
  third-party/tinygltf
)
target_link_libraries(${EXE_NAME} PRIVATE glfw)
