#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>

#include <glad/gl.h>

#include <GLFW/glfw3.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <glm/vec3.hpp>

void errorCallback(int error, const char *description) {
    std::cerr << "GLFW error " << error << ": " << description << std::endl;
}

GLFWwindow *window;

struct RaiiContext {
    RaiiContext() {
        glfwSetErrorCallback(errorCallback);

        if (!glfwInit()) {
            throw std::runtime_error("Could not initialize GLFW");
        }

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        window = glfwCreateWindow(1280, 720, "Main window", nullptr, nullptr);
        if (!window) {
            throw std::runtime_error("Could not create window");
        }

        glfwMakeContextCurrent(window);
        gladLoadGL(glfwGetProcAddress);
        glfwSwapInterval(0);

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
    RaiiFrame() {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::ShowDemoWindow();
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
        glShaderSource(idx, 1, &c_str, &len);
        glCompileShader(idx);

        str.resize(4096);
        GLsizei size = str.size();
        glGetShaderInfoLog(idx, size, &size, str.data());
        std::cout << "Compilation log:\n" << str.data() << std::endl;

        GLint status = 0;
        glGetShaderiv(idx, GL_COMPILE_STATUS, &status);
        if (!status) {
            throw std::runtime_error("Failed to compile shader");
        }
    }

    ~Shader() { clear(); }

    constexpr GLuint get() const noexcept { return idx; }

    Shader(const Shader &) = delete;
    Shader &operator=(const Shader &) = delete;

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
    ShaderProgram() : idx(0) {}

    ShaderProgram(GLuint vert, GLuint frag) {
        idx = glCreateProgram();
        glAttachShader(idx, vert);
        glAttachShader(idx, frag);
        glLinkProgram(idx);

        std::string str;
        str.resize(4096);
        GLsizei size = str.size();
        glGetProgramInfoLog(idx, size, &size, str.data());
        std::cout << "Linking log:\n" << str.data() << std::endl;

        GLint status = 0;
        glGetProgramiv(idx, GL_LINK_STATUS, &status);
        if (!status) {
            throw std::runtime_error("Failed to link program");
        }
    }

    ~ShaderProgram() { clear(); }

    constexpr GLuint get() const noexcept { return idx; }

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

GLuint vao[1];
GLuint buffers[2];

glm::vec3 vertices[] = {
    {0.0f, 0.0f, 0.0f},
    {1.0f, 0.0f, 0.0f},
    {0.0f, 1.0f, 0.0f},
};
GLushort indices[] = {0, 1, 2};

int main() {
    RaiiContext _context;

    Shader shaderVert(GL_VERTEX_SHADER, "main.vert");
    Shader shaderFrag(GL_FRAGMENT_SHADER, "main.frag");
    ShaderProgram program(shaderVert.get(), shaderFrag.get());

    glGenBuffers(2, buffers);
    glGenVertexArrays(1, vao);

    glBindVertexArray(vao[0]);
    glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices,
                 GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), nullptr);
    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    while (!glfwWindowShouldClose(window)) {
        RaiiFrame _frame;
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(program.get());
        glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[1]);
        glBindVertexArray(vao[0]);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glBindVertexArray(0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glUseProgram(0);
    }

    glDeleteVertexArrays(1, vao);
    glDeleteBuffers(2, buffers);

    return 0;
}
