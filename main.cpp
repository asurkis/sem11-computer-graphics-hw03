#include <stdexcept>
#include <iostream>

#include <glad/gl.h>

#include <GLFW/glfw3.h>

void errorCallback(int error, const char *description) {
  std::cerr << "GLFW error " << error << ": " << description << std::endl;
}

int main() {
  glfwSetErrorCallback(errorCallback);

  if (!glfwInit()) {
    throw std::runtime_error("Could not initialize GLFW");
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  GLFWwindow *window =
      glfwCreateWindow(1280, 720, "Main window", nullptr, nullptr);
  if (!window) {
    glfwTerminate();
    throw std::runtime_error("Could not create window");
  }

  glfwMakeContextCurrent(window);
  gladLoadGL(glfwGetProcAddress);
  glfwSwapInterval(0);

  while (!glfwWindowShouldClose(window)) {
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwDestroyWindow(window);
  glfwTerminate();
  return 0;
}
