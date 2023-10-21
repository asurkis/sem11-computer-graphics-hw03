#include "routine.hpp"

#include <glm/common.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_common.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/scalar_constants.hpp>
#include <glm/mat4x4.hpp>
#include <glm/trigonometric.hpp>
#include <glm/vec3.hpp>

GLuint vao[1];
GLuint buffers[2];

glm::vec3 vertices[] = {
    {-1.0f, -1.0f, -1.0f}, // 0
    {-1.0f, -1.0f, +1.0f}, // 1
    {-1.0f, +1.0f, -1.0f}, // 2
    {-1.0f, +1.0f, +1.0f}, // 3
    {+1.0f, -1.0f, -1.0f}, // 4
    {+1.0f, -1.0f, +1.0f}, // 5
    {+1.0f, +1.0f, -1.0f}, // 6
    {+1.0f, +1.0f, +1.0f}, // 7
};
GLushort indices[] = {
    0, 1, 2, 2, 1, 3, // x = -1
    4, 6, 5, 5, 6, 7, // x = +1
    0, 4, 1, 1, 4, 5, // y = -1
    2, 3, 6, 6, 3, 7, // y = +1
    0, 2, 4, 4, 2, 6, // z = -1
    1, 5, 3, 3, 5, 7, // z = +1
};

int main() {
    RaiiContext _context;

    Shader shaderVert(GL_VERTEX_SHADER, "main.vert");
    Shader shaderFrag(GL_FRAGMENT_SHADER, "main.frag");
    ShaderProgram program = ShaderProgram(shaderVert.get(), shaderFrag.get());

    GLuint uniformModel = glGetUniformLocation(program.get(), "matModel");
    GLuint uniformView = glGetUniformLocation(program.get(), "matView");
    GLuint uniformProj = glGetUniformLocation(program.get(), "matProj");

    glGenBuffers(2, buffers);
    glGenVertexArrays(1, vao);

    {
        RaiiBindBuffer _bind1(GL_ARRAY_BUFFER, buffers[0]);
        RaiiBindVao _bind2(vao[0]);
        RaiiBindBuffer _bind3(GL_ELEMENT_ARRAY_BUFFER, buffers[1]);

        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices,
                     GL_STATIC_DRAW);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices,
                     GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), 0);
    }

    while (!glfwWindowShouldClose(window)) {
        RaiiFrame _frame;

        RaiiUseProgram _bind1(program.get());
        RaiiBindBuffer _bind2(GL_ARRAY_BUFFER, buffers[0]);
        RaiiBindVao _bind3(vao[0]);
        RaiiBindBuffer _bind4(GL_ELEMENT_ARRAY_BUFFER, buffers[1]);

        int width = 0, height = 0;
        glfwGetWindowSize(window, &width, &height);
        glViewport(0, 0, width, height);

        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClearDepth(0.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        double time = glfwGetTime();
        double angle = 0.1 * time;
        glm::mat4 matModel(1.0f);
        glm::mat4 matView = glm::lookAt(
            glm::vec3{5.0 * glm::cos(angle), 5.0 * glm::sin(angle), 2.0f},
            glm::vec3{}, glm::vec3{0.0f, 0.0f, 1.0f});
        glm::mat4 matProj = glm::perspective(
            glm::radians(45.0f), 1.0f * width / height, 100.0f, 0.001f);

        glUniformMatrix4fv(uniformModel, 1, GL_FALSE,
                           reinterpret_cast<GLfloat *>(&matModel));
        glUniformMatrix4fv(uniformView, 1, GL_FALSE,
                           reinterpret_cast<GLfloat *>(&matView));
        glUniformMatrix4fv(uniformProj, 1, GL_FALSE,
                           reinterpret_cast<GLfloat *>(&matProj));

        // glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glDepthFunc(GL_GREATER);
        glDrawElements(GL_TRIANGLES, sizeof(indices) / sizeof(indices[0]), GL_UNSIGNED_SHORT, 0);
        glDisable(GL_CULL_FACE);
        // glDisable(GL_DEPTH_TEST);
    }

    glDeleteVertexArrays(1, vao);
    glDeleteBuffers(2, buffers);

    return 0;
}
