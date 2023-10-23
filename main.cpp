#include "routine.hpp"
#include <map>
#include <tuple>

#include <glm/common.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_common.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/scalar_constants.hpp>
#include <glm/geometric.hpp>
#include <glm/mat4x4.hpp>
#include <glm/trigonometric.hpp>
#include <glm/vec3.hpp>

#include <tiny_gltf.h>

constexpr float PI = glm::pi<float>();
constexpr float TWO_PI = 2.0f * PI;
constexpr float HALF_PI = 0.5f * PI;

void loadModel(tinygltf::Model &model, const std::string &path) {
    std::string err;
    std::string warn;
    tinygltf::TinyGLTF loader;
    bool ret = loader.LoadASCIIFromFile(&model, &err, &warn, path);

    std::cout << warn << '\n' << err << '\n';

    if (!ret) {
        throw std::runtime_error("Could not load model");
    }
}

void bindBufferViews(std::map<int, GLuint> &vbos,
                     const tinygltf::Model &model) {
    std::vector<bool> isVbo(model.bufferViews.size(), false);

    for (auto &mesh : model.meshes) {
        for (auto &prim : mesh.primitives) {
            auto &idxAcc = model.accessors[prim.indices];
            isVbo[idxAcc.bufferView] = true;
            for (auto &[name, accessorId] : prim.attributes) {
                auto &accessor = model.accessors[accessorId];
                isVbo[accessor.bufferView] = true;
            }
        }
    }

    for (int i = 0; i < model.bufferViews.size(); ++i) {
        auto &bufferView = model.bufferViews[i];
        if (!isVbo[i]) {
            continue;
        }
        if (bufferView.target == 0) {
            continue;
        }

        auto &buffer = model.buffers[bufferView.buffer];
        GLuint vbo = 0;
        glGenBuffers(1, &vbo);
        vbos[i] = vbo;

        RaiiBindBuffer _bind(bufferView.target, vbo);
        glBufferData(bufferView.target, bufferView.byteLength,
                     &buffer.data[bufferView.byteOffset], GL_STATIC_DRAW);
    }
}

void bindMesh(std::map<int, GLuint> &vbos, const tinygltf::Model &model,
              const tinygltf::Mesh &mesh) {
    for (auto &primitive : mesh.primitives) {
        auto &idxAccessor = model.accessors[primitive.indices];

        for (auto &[name, accessorId] : primitive.attributes) {
            int vaa = -1;
            if (name == "POSITION") {
                vaa = 0;
            } else if (name == "NORMAL") {
                vaa = 1;
            } else if (name == "TEXCOORD_0") {
                vaa = 2;
            } else {
                std::cerr << "Unknown parameter " << name << '\n';
                continue;
            }

            auto &accessor = model.accessors[accessorId];

            int size = 1;
            if (accessor.type != TINYGLTF_TYPE_SCALAR) {
                size = accessor.type;
            }

            int byteStride =
                accessor.ByteStride(model.bufferViews[accessor.bufferView]);

            glBindBuffer(GL_ARRAY_BUFFER, vbos[accessor.bufferView]);
            glEnableVertexAttribArray(vaa);
            glVertexAttribPointer(
                vaa, size, accessor.componentType,
                accessor.normalized ? GL_TRUE : GL_FALSE, byteStride,
                static_cast<char *>(nullptr) + accessor.byteOffset);
        }
    }
}

void bindModelNode(std::map<int, GLuint> &vbos, const tinygltf::Model &model,
                   const tinygltf::Node &node) {
    if (0 <= node.mesh && node.mesh < model.meshes.size()) {
        auto &mesh = model.meshes[node.mesh];
        bindMesh(vbos, model, mesh);
    }
    for (auto childId : node.children) {
        auto &child = model.nodes[childId];
        bindModelNode(vbos, model, child);
    }
}

std::tuple<GLuint, GLuint, std::map<int, GLuint>>
bindModel(const tinygltf::Model &model) {
    std::map<int, GLuint> vbos;
    GLuint vao = 0;
    glGenVertexArrays(1, &vao);
    {
        RaiiBindVao _bind(vao);
        auto &scene = model.scenes[model.defaultScene];
        bindBufferViews(vbos, model);
        for (auto nodeId : scene.nodes) {
            auto &node = model.nodes[nodeId];
            bindModelNode(vbos, model, node);
        }
    }

    for (auto it = vbos.begin(); it != vbos.end();) {
        auto &bufferView = model.bufferViews[it->first];
        if (bufferView.target == GL_ELEMENT_ARRAY_BUFFER) {
            ++it;
        } else {
            glDeleteBuffers(1, &it->second);
            vbos.erase(it++);
        }
    }

    GLuint texId = 0;
    for (auto &tex : model.textures) {
        glGenTextures(1, &texId);
        auto &img = model.images[tex.source];
        glBindTexture(GL_TEXTURE_2D, texId);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        GLenum format = 0;
        switch (img.component) {
        case 1:
            format = GL_RED;
            break;
        case 2:
            format = GL_RG;
            break;
        case 3:
            format = GL_RGB;
            break;
        case 4:
            format = GL_RGBA;
            break;
        default:
            throw std::runtime_error("Unexpected number of components");
        }

        GLenum type = 0;
        switch (img.bits) {
        case 8:
            type = GL_UNSIGNED_BYTE;
            break;
        case 16:
            type = GL_UNSIGNED_SHORT;
            break;
        default:
            throw std::runtime_error("Unsupported image format");
        }

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img.width, img.height, 0,
                     format, type, img.image.data());
        glGenerateMipmap(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, 0);
        break;
    }

    return {vao, texId, std::move(vbos)};
}

void drawModelMeshes(const std::map<int, GLuint> &ebos,
                     const tinygltf::Model &model, const tinygltf::Mesh &mesh) {
    for (auto &primitive : mesh.primitives) {
        auto &accessor = model.accessors[primitive.indices];
        RaiiBindBuffer _bind(GL_ELEMENT_ARRAY_BUFFER,
                             ebos.at(accessor.bufferView));
        glDrawElements(primitive.mode, accessor.count, accessor.componentType,
                       static_cast<char *>(nullptr) + accessor.byteOffset);
    }
}

void drawModelNodes(const std::map<int, GLuint> &ebos,
                    const tinygltf::Model &model, const tinygltf::Node &node) {
    auto &mesh = model.meshes[node.mesh];
    drawModelMeshes(ebos, model, mesh);
    for (auto &childId : node.children) {
        auto &child = model.nodes[childId];
        drawModelNodes(ebos, model, child);
    }
}

void drawModel(GLuint vao, const std::map<int, GLuint> &ebos,
               const tinygltf::Model &model) {
    RaiiBindVao _bind(vao);
    auto &scene = model.scenes[model.defaultScene];
    for (auto &nodeId : scene.nodes) {
        auto &node = model.nodes[nodeId];
        drawModelNodes(ebos, model, node);
    }
}

ShaderProgram program;

GLuint uniformSpecularPow = 0;
GLuint uniformModel = 0;
GLuint uniformView = 0;
GLuint uniformProj = 0;
GLuint uniformNormal = 0;
GLuint uniformMorphProgress = 0;
GLuint uniformDirLightDir = 0;
GLuint uniformDirLightColor = 0;
GLuint uniformSpotLightPos = 0;
GLuint uniformSpotLightDir = 0;
GLuint uniformSpotLightColor = 0;
GLuint uniformSpotLightAngleCos = 0;

void loadShaders() {
    Shader shaderVert(GL_VERTEX_SHADER, "main.vert");
    Shader shaderFrag(GL_FRAGMENT_SHADER, "main.frag");
    program = ShaderProgram(shaderVert.get(), shaderFrag.get());

    uniformSpecularPow = glGetUniformLocation(program.get(), "specularPow");
    uniformModel = glGetUniformLocation(program.get(), "matModel");
    uniformView = glGetUniformLocation(program.get(), "matView");
    uniformProj = glGetUniformLocation(program.get(), "matProj");
    uniformNormal = glGetUniformLocation(program.get(), "matNormal");
    uniformMorphProgress = glGetUniformLocation(program.get(), "morphProgress");

    uniformDirLightDir = glGetUniformLocation(program.get(), "dirLightDir");
    uniformDirLightColor = glGetUniformLocation(program.get(), "dirLightColor");

    uniformSpotLightPos = glGetUniformLocation(program.get(), "spotLightPos");
    uniformSpotLightDir = glGetUniformLocation(program.get(), "spotLightDir");
    uniformSpotLightColor =
        glGetUniformLocation(program.get(), "spotLightColor");
    uniformSpotLightAngleCos =
        glGetUniformLocation(program.get(), "spotLightAngleCos");
}

int main() {
    RaiiContext _context;

    loadShaders();

    tinygltf::Model model;
    loadModel(model, "model.gltf");
    auto [vao, texId, idxBuffers] = bindModel(model);

    glm::vec3 camPos = {0.0f, 0.0f, 5.0f};
    float camAngleX = 0.0f;
    float camAngleY = 0.0f;
    float specularPow = 16.0f;
    float morphProgress = 0.0f;
    float fov = 45.0f;
    float camSpeed = 1.0f;
    float gamma = 1.0f;

    glm::vec3 dirLightDir = {1.0f, -1.0f, -1.0f};
    glm::vec3 dirLightColor = {1.0f, 1.0f, 1.0f};
    float dirLightIntensity = 2.5f;

    glm::vec3 spotLightPos = {2.0f, 2.0f, 2.0f};
    glm::vec3 spotLightDir = {-1.0f, -1.0f, -1.0f};
    glm::vec3 spotLightColor = {0.0f, 1.0f, 1.0f};
    float spotLightPhi = 60.0f;
    float spotLightTheta = 45.0f;
    float spotLightIntensity = 5.0f;

    float rotationSpeed = 0.0f;
    float cycle = 0.0f;

    while (!glfwWindowShouldClose(window)) {
        RaiiFrame _frame;

        float deltaTime = ImGui::GetIO().DeltaTime;

        ImGui::Begin("Info");
        ImGui::Text("FPS: %f", ImGui::GetIO().Framerate);
        ImGui::SliderFloat("Camera speed", &camSpeed, 0.0f, 5.0f);
        ImGui::SliderFloat("FOV", &fov, 15.0f, 90.0f);
        ImGui::SliderFloat("Specular power", &specularPow, 0.0f, 256.0f);
        ImGui::SliderFloat("Morph progress", &morphProgress, 0.0f, 1.0f);

        if (ImGui::CollapsingHeader("Directional light")) {
            ImGui::SliderFloat("DL Direction X", &dirLightDir.x, -1.0f, 1.0f);
            ImGui::SliderFloat("DL Direction Y", &dirLightDir.y, -1.0f, 1.0f);
            ImGui::SliderFloat("DL Direction Z", &dirLightDir.z, -1.0f, 1.0f);
            ImGui::ColorEdit4("DL Color", &dirLightColor.x,
                              ImGuiColorEditFlags_NoAlpha);
            ImGui::DragFloat("DL Intensity", &dirLightIntensity);
        }

        if (ImGui::CollapsingHeader("Spot light")) {
            ImGui::DragFloat("SL Position X", &spotLightPos.x);
            ImGui::DragFloat("SL Position Y", &spotLightPos.y);
            ImGui::DragFloat("SL Position Z", &spotLightPos.z);
            ImGui::SliderFloat("SL Direction X", &spotLightDir.x, -1.0f, 1.0f);
            ImGui::SliderFloat("SL Direction Y", &spotLightDir.y, -1.0f, 1.0f);
            ImGui::SliderFloat("SL Direction Z", &spotLightDir.z, -1.0f, 1.0f);
            ImGui::ColorEdit4("SL Color", &spotLightColor.x,
                              ImGuiColorEditFlags_NoAlpha);
            ImGui::DragFloat("SL Phi", &spotLightPhi);
            ImGui::DragFloat("SL Theta", &spotLightTheta);
            ImGui::DragFloat("SL Intensity", &spotLightIntensity);
        }

        ImGui::SliderFloat("Model rotation speed", &rotationSpeed, 0.0f, 1.0f);

        if (ImGui::Button("Reload shaders")) {
            try {
                loadShaders();
            } catch (...) {
                // Failed to compile shaders
            }
        }
        ImGui::End();

        int width = 0, height = 0;
        glfwGetWindowSize(window, &width, &height);
        glViewport(0, 0, width, height);

        float fovTan = glm::tan(glm::radians(fov));

        // Calculate camera direction
        ImVec2 mouseDelta;
        if (!ImGui::GetIO().WantCaptureMouse) {
            mouseDelta = ImGui::GetMouseDragDelta(0, 0.0f);
            ImGui::ResetMouseDragDelta();
        }
        camAngleX += fovTan * mouseDelta.y / height;
        camAngleY -= fovTan * mouseDelta.x / height;
        camAngleX = glm::clamp(camAngleX, -HALF_PI, HALF_PI);
        camAngleY = TWO_PI * glm::fract(camAngleY / TWO_PI);

        // Calculate camera movement
        glm::vec3 camRight = {glm::cos(camAngleY), 0.0f, glm::sin(camAngleY)};
        glm::vec3 camForward =
            glm::vec3{camRight.z, 0.0f, -camRight.x} * glm::cos(camAngleX) +
            glm::vec3{0.0f, glm::sin(camAngleX), 0.0f};
        glm::vec3 camUp = glm::cross(camRight, camForward);

        if (ImGui::IsKeyDown(ImGuiKey_W)) {
            camPos += deltaTime * camSpeed * camForward;
        }
        if (ImGui::IsKeyDown(ImGuiKey_S)) {
            camPos -= deltaTime * camSpeed * camForward;
        }
        if (ImGui::IsKeyDown(ImGuiKey_D)) {
            camPos += deltaTime * camSpeed * camRight;
        }
        if (ImGui::IsKeyDown(ImGuiKey_A)) {
            camPos -= deltaTime * camSpeed * camRight;
        }
        if (ImGui::IsKeyDown(ImGuiKey_E)) {
            camPos.y += deltaTime * camSpeed;
        }
        if (ImGui::IsKeyDown(ImGuiKey_Q)) {
            camPos.y -= deltaTime * camSpeed;
        }

        glClearColor(1.0f, 0.75f, 0.5f, 1.0f);
        glClearDepth(0.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        cycle += rotationSpeed * deltaTime;
        cycle = glm::fract(cycle);
        float rotAngle = TWO_PI * glm::smoothstep(0.0f, 1.0f, cycle);
        glm::mat4 matModel =
            glm::rotate(glm::mat4(1.0f), rotAngle, {1.0f, 0.0f, 0.0f});

        glm::mat4 matView(1.0f);
        matView[0] = glm::vec4(camRight, 0.0f);
        matView[1] = glm::vec4(camUp, 0.0f);
        matView[2] = glm::vec4(-camForward, 0.0f);
        matView = glm::transpose(matView);
        matView = glm::translate(matView, -camPos);
        glm::mat4 matProj = glm::perspective(
            glm::radians(fov), 1.0f * width / height, 100.0f, 0.001f);

        glm::mat4 matNormal = glm::transpose(glm::inverse(matView * matModel));

        RaiiUseProgram _bind1(program.get());
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE,
                           reinterpret_cast<GLfloat *>(&matModel));
        glUniformMatrix4fv(uniformView, 1, GL_FALSE,
                           reinterpret_cast<GLfloat *>(&matView));
        glUniformMatrix4fv(uniformProj, 1, GL_FALSE,
                           reinterpret_cast<GLfloat *>(&matProj));
        glUniformMatrix4fv(uniformNormal, 1, GL_FALSE,
                           reinterpret_cast<GLfloat *>(&matNormal));
        glUniform1f(uniformSpecularPow, specularPow);
        glUniform1f(uniformMorphProgress, morphProgress);

        // We will calculate everything in view space,
        // where coordinates are still orthonormal
        matNormal = glm::transpose(glm::inverse(matView));
        glm::vec4 dlDir = matNormal * glm::vec4(glm::normalize(dirLightDir), 0);
        glm::vec3 dlColor = dirLightIntensity * dirLightColor;

        glm::vec4 slPos = matView * glm::vec4(spotLightPos, 1);
        glm::vec4 slDir =
            matNormal * glm::vec4(glm::normalize(spotLightDir), 0);
        glm::vec2 slAngle =
            glm::cos(glm::radians(glm::vec2{spotLightPhi, spotLightTheta}));
        glm::vec3 slColor = spotLightIntensity * spotLightColor;

        glUniform3f(uniformDirLightDir, dlDir.x, dlDir.y, dlDir.z);
        glUniform3f(uniformDirLightColor, dlColor.x, dlColor.y, dlColor.z);

        glUniform3f(uniformSpotLightPos, slPos.x, slPos.y, slPos.z);
        glUniform3f(uniformSpotLightDir, slDir.x, slDir.y, slDir.z);
        glUniform3f(uniformSpotLightColor, slColor.x, slColor.y, slColor.z);
        glUniform2f(uniformSpotLightAngleCos, slAngle.x, slAngle.y);

        glEnable(GL_MULTISAMPLE);
        glEnable(GL_DEPTH_TEST);
        // glEnable(GL_CULL_FACE);

        glDepthFunc(GL_GREATER);
        glBindTexture(GL_TEXTURE_2D, texId);
        drawModel(vao, idxBuffers, model);
        glBindTexture(GL_TEXTURE_2D, 0);

        // glDisable(GL_CULL_FACE);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_MULTISAMPLE);
    }

    glDeleteVertexArrays(1, &vao);
    glDeleteTextures(1, &texId);
    // glDeleteBuffers(2, buffers);
    for (auto [_pos, idx] : idxBuffers) {
        glDeleteBuffers(1, &idx);
    }

    return 0;
}
