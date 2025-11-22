#include <cmath>
#include <filesystem>
#define GLM_ENABLE_EXPERIMENTAL
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
#include "sprite.cpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

constexpr bool DRAW_DEBUG = true;

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
        LTRACE("Game::Game() success!");
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
        for (Sprite* sprite : sprites) { sprite->draw(camera.get_view_projection()); }
        Ship::predraw(*_ship_draw_data);
        for (Ship* ship : ships) { ship->draw(*_ship_draw_data, camera.get_view_projection()); }
    }
    inline void debug_draw() {
        spdlog::default_logger()->flush();
        Ship::predraw_debug(*_ship_draw_data);
        for (Ship* ship : ships) { ship->draw_debug(*_ship_draw_data, camera.get_view_projection()); }
    }

    std::vector<Ship*> ships{};
    // TODO: associating sprites with Texture for using Sprite::predraw() only once per frame
    std::vector<Sprite*> sprites{};

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
    game->sprites.push_back(new Sprite("assets/wall02.png", game->resource_manager, {2.0f, 2.0f}, {0.0f, 128.0f}));

    {
        int w, h;
        glfwGetWindowSize(window, &w, &h);
        game->_set_viewport_dimensions(w, h);
    }
    glfwSetFramebufferSizeCallback(window, [](GLFWwindow* win, int w, int h) { Game::_get(win)->_set_viewport_dimensions(w, h); });
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glfwSwapInterval(0);
    double last_frame = glfwGetTime(), frame_delta = 0;
    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT);
        game->process_input();
        game->process_physics(frame_delta);
        game->draw();
        if (DRAW_DEBUG) game->debug_draw();
        glfwSwapBuffers(window);
        const double now = glfwGetTime();
        frame_delta = now - last_frame;
        last_frame = now;
    }
    glfwDestroyWindow(window);
}
