#include <cmath>
#include <filesystem>
#include <glm/gtc/constants.hpp>
#include <glm/trigonometric.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/vector_float2_precision.hpp>
#include <glm/geometric.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <glm/vec2.hpp>
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GLFW/glfw3.h>
#include <spdlog/common.h>
#include <spdlog/spdlog.h>
#include <stdio.h>

#include <cstddef>

#include "input.cpp"
#include "log.cpp"
#include "mesh.cpp"
#include "resource_manager.cpp"
#include "shader.cpp"
#include "shaders.hpp"
#include "texture.cpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
// px/s^2
constexpr double ACCELERATION = 100.0;

class Camera {
    glm::vec2 _dimensions{};

public:
    glm::vec2 pos{};
    glm::mat4x4 projection = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 1.0f);
    glm::mat4x4 get_view_projection() {
        glm::mat4x4 view{1.0f};

        view[3] = glm::vec4(pos, 0.0f, 1.0f);
        return projection * view;
    }
    inline void set_dimensions(const uint w, const uint h) {
        _dimensions.x = w;
        _dimensions.y = h;
        const float aspect = _dimensions.x / _dimensions.y;
        // projection = glm::ortho(-1.0f, aspect, -1.0f, 1.0f / aspect, 0.0f, 1.0f);
        projection = glm::ortho(-_dimensions.x / 2.0f, _dimensions.x / 2.0f, -_dimensions.y / 2.0f, _dimensions.y / 2.0f, 0.0f, 1.0f);
    }
    inline glm::vec2 get_dimensions() const { return _dimensions; }
};
class Ship {
public:
    struct _StaticDrawResources {
        std::shared_ptr<Shader> shader;
        std::shared_ptr<Shader> white_shader;
        std::shared_ptr<Mesh> mesh;
        _StaticDrawResources(ResourceManager& manager) {
            shader = manager.get_shader(VERTEX_SHADER_2D, FRAGMENT_SHADER_2D);
            white_shader = manager.get_shader(VERTEX_SHADER_2D, FRAGMENT_SHADER_2D_SINGLE_COLOR);
            mesh = manager.get_mesh_rect(0.5f, 0.5f, 1.0f, 1.0f);
        }
    };
    glm::vec2 pos{};
    glm::vec2 vel{};
    // deprecated in favour of Mesh::pivot
    // glm::vec2 pivot{0.0f, 0.0f};
    double angle{};
    std::shared_ptr<Texture> sprite;
    glm::ivec2 dimensions;

    glm::mat4x4 get_model() {
        // create identity matrix
        glm::mat4x4 out{1.0f};
        // set position
        out[3] = glm::vec4(pos, 0.0f, 1.0f);
        // rotate it
        // out = glm::translate(glm::mat4x4(1.0f), glm::vec3(pivot, 0.0f)) * glm::rotate(out, float(angle), glm::vec3(0.0f, 0.0f, 1.0f)) *
        // glm::translate(glm::mat4x4(1.0f), glm::vec3(-pivot, 0.0f));
        out = glm::rotate(out, float(angle), glm::vec3(0.0f, 0.0f, 1.0f));
        // scale it
        out = glm::scale(out, glm::vec3(dimensions, 0.0f));
        return out;
    }
    // once per frame
    static void predraw(_StaticDrawResources& res) {
        res.shader->use();
        res.mesh->use();
    }
    static void predraw_debug(_StaticDrawResources& res) { res.white_shader->use(); }
    // n times per frame
    void draw(_StaticDrawResources& res, const glm::mat4x4& VP) {
        res.shader->set_mat4("MVP", VP * get_model());
        sprite->use(0);
        res.mesh->draw();
    }
    void draw_debug(_StaticDrawResources& res, const glm::mat4x4& VP) {
        res.shader->set_mat4("MVP", VP * get_model());
        sprite->use(0);
        res.mesh->draw_lines();
    }
    void physics(const double dt, Input& input) {
        glm::vec2 input_dir = glm::vec2((input.RIGHT - input.LEFT), (input.FORWARD - input.BACKWARD));
        const glm::vec2& mpos = input.mouse_world_pos;
        angle = glm::atan(mpos.y - pos.y, mpos.x - pos.x) - glm::half_pi<float>();
        if (input_dir != glm::vec2())
            vel += (glm::vec2(glm::rotate(float(angle), glm::vec3{0.0f, 0.0f, 1.0f}) * glm::vec4(input_dir * float(dt * ACCELERATION), 0.0f, 0.0f)));
        else if (glm::length(vel) <= ACCELERATION * dt * dt)
            vel = glm::vec2(0.0f, 0.0f);
        else
            vel -= glm::normalize(vel) * float(.4 * dt * dt);
        pos += vel * float(dt);
    }
    Ship(std::shared_ptr<Texture> sprite, glm::vec2 pos = glm::vec2{0.0f, 0.0f}, double rot = 0.0)
        : sprite(sprite), pos(pos), angle(rot), dimensions(sprite->w(), sprite->h()) {}
};

class Game {
    GLFWwindow* _window;

    Input input{};

    Ship::_StaticDrawResources* _ship_draw_data;
    Camera camera{};

public:
    inline static Game* _cast(void* ptr) { return static_cast<Game*>(ptr); }
    inline static Game* _get(GLFWwindow* window) { return _cast(glfwGetWindowUserPointer(window)); }
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
            _get(w)->input.key_cb(key, action == GLFW_PRESS, mods, _get(w));
        });
        glfwSetMouseButtonCallback(_window,
                                   [](GLFWwindow* w, int button, int action, int mods) { _get(w)->input.mouse_cb(button, action == GLFW_PRESS, _get(w)); });

        glfwMakeContextCurrent(_window);
        glEnable(GL_BLEND);
        resource_manager.get_texture("assets/ship01.png");
        _ship_draw_data = new Ship::_StaticDrawResources(resource_manager);
        LTRACE("SUCCESS #2");
    }

    GLFWwindow* get_window() const { return _window; }

    inline void process_input() {
        glfwPollEvents();

        double mousex, mousey;
        glfwGetCursorPos(_window, &mousex, &mousey);
        input.mouse_screen_pos = glm::vec2(mousex, mousey);
        input.mouse_world_pos = camera.pos - camera.get_dimensions() / 2.0f + glm::vec2(mousex, mousey);
        input.mouse_world_pos.y = -input.mouse_world_pos.y;
    }
    inline void process_physics(const double& delta) {
        for (Ship* ship : ships) { ship->physics(delta, input); }
    }
    inline void draw() {
        spdlog::default_logger()->flush();
        Ship::predraw(*_ship_draw_data);
        for (Ship* ship : ships) { ship->draw(*_ship_draw_data, camera.get_view_projection()); }
        // Ship::predraw_debug(*_ship_draw_data);
        // for (Ship* ship : ships) { ship->draw_debug(*_ship_draw_data, camera.get_view_projection()); }
    }
    inline void debug_draw() {
        spdlog::default_logger()->flush();
        Ship::predraw_debug(*_ship_draw_data);
        for (Ship* ship : ships) { ship->draw_debug(*_ship_draw_data, camera.get_view_projection()); }
    }
    std::vector<Ship*> ships{};
    inline void _set_viewport_dimensions(const uint w, const uint h) {
        camera.set_dimensions(w, h);
        glViewport(0, 0, w, h);
    }
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
    GLFWwindow* window = glfwCreateWindow(640, 640, PROJECT_NAME_VERSION, NULL, NULL);
    if (!window) LCRITRET(1, "!window");
    Game* game = new Game(window);
    game->ships.push_back(new Ship(game->resource_manager.get_texture("assets/ship01.png")));

    glfwSetFramebufferSizeCallback(window, [](GLFWwindow* win, int w, int h) { Game::_get(win)->_set_viewport_dimensions(w, h); });
    game->_set_viewport_dimensions(640, 640);

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    double last_frame = glfwGetTime(), frame_delta = 0;
    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT);
        game->process_input();
        game->process_physics(frame_delta);
        game->draw();
        game->debug_draw();
        // game->post_render();
        glfwSwapBuffers(window);
        const double now = glfwGetTime();
        frame_delta = now - last_frame;
        last_frame = now;
    }
    glfwDestroyWindow(window);
}
