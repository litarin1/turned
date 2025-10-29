#include <glm/ext/vector_float2_precision.hpp>
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GLFW/glfw3.h>
#include <spdlog/common.h>
#include <spdlog/spdlog.h>
#include <stdio.h>

#include <cstddef>
#include <glm/vec2.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/rotate_vector.hpp>

#include "input.cpp"
#include "log.cpp"
#include "mesh.cpp"
#include "shader.cpp"

constexpr static const char* VERTEX_SHADER_2D = R"(
#version 330 core
layout (location = 0) in vec2 aPos;
out vec4 vertexColor;
void main(){
    gl_Position = vec4(aPos, 0.0, 1.0);
    vertexColor = vec4(abs(aPos.x), abs(aPos.x), abs(aPos.x), 1.0);
    // vertexColor = vec4(1.0, 1.0, 1.0, 1.0); // white
}
)";
constexpr static const char* FRAGMENT_SHADER_2D = R"(
#version 330 core
out vec4 Color;
in vec4 vertexColor;
void main(){
    Color = vertexColor;
}
)";

class Ship {
public:
    struct _StaticDrawResources {
        Shader shader{VERTEX_SHADER_2D, FRAGMENT_SHADER_2D};
        Mesh mesh = Mesh::construct_rect(1.0f, 1.0f);
        _StaticDrawResources() { LTRACE("SUCCESS #1"); }
    };
    glm::vec2 pos{};
    double rot{};
    // once per frame
    static void predraw(_StaticDrawResources& res) {
        res.shader.use();
        res.mesh.use();
    }
    // n times per frame
    void draw(_StaticDrawResources& res) { res.mesh.draw(); }
    void physics(const double dt, Input& input) {
        pos += glm::vec2(glm::rotate(float(rot), glm::vec3{0.0f, 0.0f, 1.0f}) * glm::vec4((input.FORWARD - input.BACKWARD) * dt, 0.0f, 0.0f, 0.0f));
    }
    Ship(glm::vec2 pos = glm::vec2{0.0f, 0.0f}, double rot = 0.0) : pos(pos), rot(rot) {}
};

class Game {
    GLFWwindow* _window;

    Input input{};

    inline static Game* _cast(void* ptr) { return static_cast<Game*>(ptr); }
    inline static Game* _get(GLFWwindow* window) { return _cast(glfwGetWindowUserPointer(window)); }
    Ship::_StaticDrawResources* _ship_draw_data;

public:
    Game(GLFWwindow* window) : _window(window) {
        glfwSetWindowUserPointer(_window, this);

        input.QUIT = [](void* _this) {
            LINFO("QUIT");
            glfwSetWindowShouldClose(_cast(_this)->_window, GLFW_TRUE);
        };
        input.PRINT_HELO = [](void* _this) { LINFO("HELO!!"); };

        glfwSetKeyCallback(_window, [](GLFWwindow* w, int key, int scancode, int action, int mods) {
            if (action == GLFW_REPEAT) return;
            _get(w)->input.key_cb(key, true ? action == GLFW_PRESS : false, mods, _get(w));
        });

        glfwMakeContextCurrent(_window);
        _ship_draw_data = new Ship::_StaticDrawResources();
        LTRACE("SUCCESS #2");
    }

    GLFWwindow* get_window() const { return _window; }

    inline void process_input() { glfwPollEvents(); }
    inline void process_physics(const double& delta) {
        for (Ship* ship : ships) { ship->physics(delta, input); }
    }
    inline void draw() {
        spdlog::default_logger()->flush();
        Ship::predraw(*_ship_draw_data);
        for (Ship* ship : ships) { ship->draw(*_ship_draw_data); }
    }
    std::vector<Ship*> ships{};
};
int main() {
    _init_log();
    spdlog::set_level(spdlog::level::trace);

    LDEBUG("{}", PROJECT_NAME_VERSION);
    if (!glfwInit()) LCRITRET(1, "!glfwInit()");
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    // TODO: dynamic window size
    GLFWwindow* window = glfwCreateWindow(640, 640, PROJECT_NAME_VERSION, NULL, NULL);
    if (!window) LCRITRET(1, "!window");
    Game* game = new Game(window);
    game->ships.push_back(new Ship());

    // TODO: dynamic window size
    glViewport(0, 0, 640, 640);

    double last_frame = glfwGetTime(), frame_delta = 0;
    while (!glfwWindowShouldClose(window)) {
        game->process_input();
        game->process_physics(frame_delta);
        game->draw();
        // game->post_render();
        glfwSwapBuffers(window);
        const double now = glfwGetTime();
        frame_delta = now - last_frame;
        last_frame = now;
        glfwSwapBuffers(window);
    }
    glfwDestroyWindow(window);
}
