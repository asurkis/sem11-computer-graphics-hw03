#ifndef ROUTINE_H
#define ROUTINE_H

#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>

#include <glad/gl.h>

#include <GLFW/glfw3.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

inline void errorCallback(int error, const char *description) {
    std::cerr << "GLFW error " << error << ": " << description << std::endl;
}

inline void APIENTRY glDebugOutput(GLenum source, GLenum type, unsigned int id,
                            GLenum severity, GLsizei length,
                            const char *message, const void *userParam) {
    // ignore non-significant error/warning codes
    if (id == 131169 || id == 131185 || id == 131218 || id == 131204) return;

    std::cout << "---------------" << std::endl;
    std::cout << "Debug message (" << id << "): " << message << std::endl;

    switch (source) {
    case GL_DEBUG_SOURCE_API: std::cout << "Source: API"; break;
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
        std::cout << "Source: Window System";
        break;
    case GL_DEBUG_SOURCE_SHADER_COMPILER:
        std::cout << "Source: Shader Compiler";
        break;
    case GL_DEBUG_SOURCE_THIRD_PARTY: std::cout << "Source: Third Party"; break;
    case GL_DEBUG_SOURCE_APPLICATION: std::cout << "Source: Application"; break;
    case GL_DEBUG_SOURCE_OTHER: std::cout << "Source: Other"; break;
    }
    std::cout << std::endl;

    switch (type) {
    case GL_DEBUG_TYPE_ERROR: std::cout << "Type: Error"; break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
        std::cout << "Type: Deprecated Behaviour";
        break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
        std::cout << "Type: Undefined Behaviour";
        break;
    case GL_DEBUG_TYPE_PORTABILITY: std::cout << "Type: Portability"; break;
    case GL_DEBUG_TYPE_PERFORMANCE: std::cout << "Type: Performance"; break;
    case GL_DEBUG_TYPE_MARKER: std::cout << "Type: Marker"; break;
    case GL_DEBUG_TYPE_PUSH_GROUP: std::cout << "Type: Push Group"; break;
    case GL_DEBUG_TYPE_POP_GROUP: std::cout << "Type: Pop Group"; break;
    case GL_DEBUG_TYPE_OTHER: std::cout << "Type: Other"; break;
    }
    std::cout << std::endl;

    switch (severity) {
    case GL_DEBUG_SEVERITY_HIGH: std::cout << "Severity: high"; break;
    case GL_DEBUG_SEVERITY_MEDIUM: std::cout << "Severity: medium"; break;
    case GL_DEBUG_SEVERITY_LOW: std::cout << "Severity: low"; break;
    case GL_DEBUG_SEVERITY_NOTIFICATION:
        std::cout << "Severity: notification";
        break;
    }
    std::cout << std::endl;
    std::cout << std::endl;
}

inline GLFWwindow *window;

struct RaiiContext {
    RaiiContext(const RaiiContext &) = delete;
    RaiiContext &operator=(const RaiiContext &) = delete;

    RaiiContext() {
        glfwSetErrorCallback(errorCallback);

        if (!glfwInit()) {
            throw std::runtime_error("Could not initialize GLFW");
        }

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
        glfwWindowHint(GLFW_SAMPLES, 16);

        window = glfwCreateWindow(1280, 720, "Main window", nullptr, nullptr);
        if (!window) { throw std::runtime_error("Could not create window"); }

        glfwMakeContextCurrent(window);
        gladLoadGL(glfwGetProcAddress);
        glfwSwapInterval(0);

        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(glDebugOutput, nullptr);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0,
                              nullptr, GL_TRUE);

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO &io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init();
    }

    ~RaiiContext() {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

        glfwDestroyWindow(window);
        glfwTerminate();
    }
};

struct RaiiFrame {
    RaiiFrame(const RaiiFrame &) = delete;
    RaiiFrame &operator=(const RaiiFrame &) = delete;

    RaiiFrame() {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }

    ~RaiiFrame() {
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);

        glfwPollEvents();
        if (ImGui::IsKeyPressed(ImGuiKey::ImGuiKey_Escape)) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }
    }
};

struct Shader {
    Shader(const Shader &) = delete;
    Shader &operator=(const Shader &) = delete;

    Shader() : idx(0) {}

    Shader(GLenum type, const char *filepath) {
        std::ifstream ifs(filepath);
        std::ostringstream oss;
        oss << ifs.rdbuf();
        std::string str = oss.str();

        std::cout << "Compiling shader " << filepath
                  << " with source code:\n"
                     "////////////////////////////////\n"
                  << str << "////////////////////////////////\n";

        const GLchar *c_str = str.c_str();
        GLint len = str.size();

        idx = glCreateShader(type);
        glShaderSource(get(), 1, &c_str, &len);
        glCompileShader(get());

        str.resize(4096);
        GLsizei size = str.size();
        glGetShaderInfoLog(get(), size, &size, str.data());
        std::cout << "Compilation log:\n" << str.data() << std::endl;

        GLint status = 0;
        glGetShaderiv(get(), GL_COMPILE_STATUS, &status);
        if (!status) { throw std::runtime_error("Failed to compile shader"); }
    }

    ~Shader() { clear(); }

    constexpr GLuint get() const noexcept { return idx; }

    Shader(Shader &&rhs) : idx(rhs.release()) {}
    Shader &operator=(Shader &&rhs) {
        clear();
        idx = rhs.release();
        return *this;
    }

  private:
    GLuint idx = 0;

    GLuint release() {
        GLuint res = idx;
        idx = 0;
        return res;
    }

    void clear() {
        glDeleteShader(idx);
        idx = 0;
    }
};

struct ShaderProgram {
    ShaderProgram(const ShaderProgram &) = delete;
    ShaderProgram &operator=(const ShaderProgram &) = delete;

    ShaderProgram() : idx(0) {}

    ShaderProgram(GLuint vert, GLuint frag) {
        idx = glCreateProgram();
        glAttachShader(get(), vert);
        glAttachShader(get(), frag);
        glLinkProgram(get());

        std::string str;
        str.resize(4096);
        GLsizei size = str.size();
        glGetProgramInfoLog(get(), size, &size, str.data());
        std::cout << "Linking log:\n" << str.data() << std::endl;

        GLint status = 0;
        glGetProgramiv(get(), GL_LINK_STATUS, &status);
        if (!status) { throw std::runtime_error("Failed to link program"); }
    }

    ~ShaderProgram() { clear(); }

    constexpr GLuint get() const noexcept { return idx; }

    ShaderProgram(ShaderProgram &&rhs) : idx(rhs.release()) {}
    ShaderProgram &operator=(ShaderProgram &&rhs) {
        clear();
        idx = rhs.release();
        return *this;
    }

  private:
    GLuint idx = 0;

    GLuint release() {
        GLuint res = idx;
        idx = 0;
        return res;
    }

    void clear() {
        glDeleteProgram(idx);
        idx = 0;
    }
};

struct RaiiUseProgram {
    RaiiUseProgram(const RaiiUseProgram &) = delete;
    RaiiUseProgram &operator=(const RaiiUseProgram &) = delete;

    RaiiUseProgram(GLuint idx) { glUseProgram(idx); }
    ~RaiiUseProgram() { glUseProgram(0); }
};

struct RaiiBindBuffer {
    RaiiBindBuffer(const RaiiBindBuffer &) = delete;
    RaiiBindBuffer &operator=(const RaiiBindBuffer &) = delete;

    RaiiBindBuffer(GLenum target, GLuint idx) : target(target) {
        glBindBuffer(target, idx);
    }
    ~RaiiBindBuffer() { glBindBuffer(target, 0); }

  private:
    GLenum target;
};

struct RaiiBindTexture {
    RaiiBindTexture(const RaiiBindTexture &) = delete;
    RaiiBindTexture &operator=(const RaiiBindTexture &) = delete;

    RaiiBindTexture(GLenum target, GLuint idx) : target(target) {
        glBindTexture(target, idx);
    }
    ~RaiiBindTexture() { glBindTexture(target, 0); }

  private:
    GLenum target;
};

struct RaiiBindVao {
    RaiiBindVao(const RaiiBindVao &) = delete;
    RaiiBindVao &operator=(const RaiiBindVao &) = delete;

    RaiiBindVao(GLuint idx) { glBindVertexArray(idx); }
    ~RaiiBindVao() { glBindVertexArray(0); }
};

#endif
