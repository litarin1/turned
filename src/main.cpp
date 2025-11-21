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
#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GLFW/glfw3.h>
#include <spdlog/common.h>
#include <spdlog/spdlog.h>
#include <stdio.h>

#include <cstddef>

#include "camera.cpp"
#include "input.cpp"
#include "log.cpp"
#include "resource_manager.cpp"
#include "ship.cpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

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
    glfwSwapInterval(0);
    double last_frame = glfwGetTime(), frame_delta = 0;
    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT);
        game->process_input();
        game->process_physics(frame_delta);
        game->draw();
        // game->debug_draw();
        // game->post_render();
        glfwSwapBuffers(window);
        const double now = glfwGetTime();
        frame_delta = now - last_frame;
        last_frame = now;
    }
    glfwDestroyWindow(window);
}
