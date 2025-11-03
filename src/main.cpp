#include <filesystem>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/vector_float2_precision.hpp>
#include <glm/geometric.hpp>
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
#include "resource_manager.cpp"
#include "shader.cpp"
#include "texture.cpp"

constexpr static const char* VERTEX_SHADER_2D = R"(
#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoord;
uniform mat4 MVP;
out vec4 vertexColor;
out vec2 TexCoord;
void main(){
    // gl_Position = vec4(aPos, 0.0, 1.0);
    gl_Position = MVP * vec4(aPos, 0.0, 1.0);

    vertexColor = vec4(abs(aPos.x), abs(aPos.x), abs(aPos.x), 1.0);
    TexCoord = aTexCoord;
    // vertexColor = vec4(1.0, 1.0, 1.0, 1.0); // white
}
)";
constexpr static const char* FRAGMENT_SHADER_2D = R"(
#version 330 core
out vec4 Color;
in vec4 vertexColor;
in vec2 TexCoord;
uniform sampler2D Sprite;
void main(){
    Color = texture(Sprite, TexCoord);
    if (Color.a != 1.0) discard;
}
)";
constexpr uint WINDOW_WIDTH = 640;
constexpr uint WINDOW_HEIGHT = 640;
constexpr float WINDOW_ASPECT = float(WINDOW_WIDTH) / float(WINDOW_HEIGHT);
class Camera {
public:
    glm::vec2 pos{};
    glm::mat4x4 projection = glm::ortho(-1.0f, WINDOW_ASPECT, -1.0f, 1.0f / WINDOW_ASPECT, 0.0f, 1.0f);
    glm::mat4x4 get_view_projection() {
        glm::mat4x4 view{1.0f};

        view[3] = glm::vec4(pos, 0.0f, 1.0f);
        return projection * view;
    }
};
class Ship {
public:
    struct _StaticDrawResources {
        std::shared_ptr<Shader> shader;
        std::shared_ptr<Mesh> mesh;
        _StaticDrawResources(ResourceManager& manager) {
            shader = manager.get_shader(VERTEX_SHADER_2D, FRAGMENT_SHADER_2D);
            mesh = manager.get_mesh_rect(1.0f, 1.0f);
        }
    };
    glm::vec2 pos{};
    glm::vec2 vel{};
    glm::vec2 pivot{0.5f, 0.5f};
    double angle{};
    std::shared_ptr<Texture> sprite;

    glm::mat4x4 get_model() {
        // create identity matrix
        glm::mat4x4 out{1.0f};
        // set position
        out[3] = glm::vec4(pos, 0.0f, 1.0f);
        // rotate it
        out = glm::translate(glm::mat4x4(1.0f), glm::vec3(pivot, 0.0f)) * glm::rotate(out, float(angle), glm::vec3(0.0f, 0.0f, 1.0f)) *
              glm::translate(glm::mat4x4(1.0f), glm::vec3(-pivot, 0.0f));
        return out;
    }
    // once per frame
    static void predraw(_StaticDrawResources& res) {
        res.shader->use();
        res.mesh->use();
    }
    // n times per frame
    void draw(_StaticDrawResources& res, const glm::mat4x4& VP) {
        res.shader->set_mat4("MVP", VP * get_model());
        sprite->use(0);
        res.mesh->draw();
    }
    void physics(const double dt, Input& input) {
        glm::vec2 input_dir = glm::vec2((input.RIGHT - input.LEFT), (input.FORWARD - input.BACKWARD));
        angle += (input.TURN_LEFT - input.TURN_RIGHT) * dt;
        // pos += glm::vec2(glm::rotate(float(rot), glm::vec3{0.0f, 0.0f, 1.0f}) * glm::vec4(input_dir, 0.0f, 0.0f));
        if (input_dir != glm::vec2())
            vel += glm::vec2(glm::rotate(float(angle), glm::vec3{0.0f, 0.0f, 1.0f}) * glm::vec4(input_dir * float(dt * dt), 0.0f, 0.0f));
        else if (glm::length(vel) <= .4f * dt * dt)
            vel = glm::vec2(0.0f, 0.0f);
        else
            vel -= glm::normalize(vel) * float(.4 * dt * dt);
        pos += vel;
    }
    Ship(std::shared_ptr<Texture> sprite, glm::vec2 pos = glm::vec2{0.0f, 0.0f}, double rot = 0.0) : sprite(sprite), pos(pos), angle(rot) {}
};

class Game {
    GLFWwindow* _window;

    Input input{};

    inline static Game* _cast(void* ptr) { return static_cast<Game*>(ptr); }
    inline static Game* _get(GLFWwindow* window) { return _cast(glfwGetWindowUserPointer(window)); }
    Ship::_StaticDrawResources* _ship_draw_data;
    Camera camera{};

public:
    ResourceManager resource_manager{};
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
        glEnable(GL_BLEND);
        resource_manager.get_texture("ship01.png");
        _ship_draw_data = new Ship::_StaticDrawResources(resource_manager);
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
        for (Ship* ship : ships) { ship->draw(*_ship_draw_data, camera.get_view_projection()); }
    }
    std::vector<Ship*> ships{};
};
int main() {
    std::filesystem::current_path(std::filesystem::canonical("/proc/self/exe").parent_path());
    LINFO(std::filesystem::current_path().c_str());
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
    game->ships.push_back(new Ship(game->resource_manager.get_texture("ship01.png")));

    // TODO: dynamic window size
    glViewport(0, 0, 640, 640);

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    double last_frame = glfwGetTime(), frame_delta = 0;
    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT);
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
