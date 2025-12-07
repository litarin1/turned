#include <box2d/types.h>

#include <cmath>
#include <filesystem>

#define GLM_ENABLE_EXPERIMENTAL
#define GL_GLEXT_PROTOTYPES
#define DRAW_DEBUG
#include <GL/gl.h>
#include <GLFW/glfw3.h>
#include <box2d/box2d.h>
#include <spdlog/common.h>
#include <spdlog/spdlog.h>
#include <stdio.h>

#include <cstddef>

#include "camera.cpp"
#include "globals.hpp"
#include "input.cpp"
#include "log.cpp"
#include "resource_manager.cpp"
#include "ship.cpp"
#include "sprite.cpp"
#include "static_body.cpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

class Game {
    GLFWwindow* _window;

    Input input{};

public:
    ResourceManager resource_manager{};

private:
    Camera camera{};
    b2WorldId world_id;

    Sprite::_StaticDrawResources _sprite_resources;

public:
    // TODO: current_controller so it can use not only the ship but the polymorphic controller
    Ship* current_ship = nullptr;
    inline b2WorldId& get_world() { return world_id; }
    inline static Game* _cast(void* ptr) { return static_cast<Game*>(ptr); }
    inline static Game* _get(GLFWwindow* window) { return _cast(glfwGetWindowUserPointer(window)); }

    Game(GLFWwindow* window, b2WorldDef& world_def) : _window(window), world_id(b2CreateWorld(&world_def)), _sprite_resources(resource_manager) {
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

        glEnable(GL_BLEND);
        std::ignore = resource_manager.get_texture("assets/ship01.png");
        LTRACE("Game::Game() success!");
    }

    GLFWwindow* get_window() const { return _window; }

    inline void process_input() {
        glfwPollEvents();

        double mousex, mousey;
        glfwGetCursorPos(_window, &mousex, &mousey);
        input.mouse_screen_pos = glm::vec2(mousex, mousey);
        input.mouse_world_pos = (camera.pos / float(ZOOM_FACTOR) - camera.get_dimensions() / 2.0f + glm::vec2(mousex, mousey) / float(ZOOM_FACTOR));
        input.mouse_world_pos.y = -input.mouse_world_pos.y;

        if (current_ship) {
            const double&& angle = oriented_angle_between(input.mouse_world_pos - current_ship->get_transform().pos, glm::vec2(0.0f, 1.0f));
            current_ship->inputs = {input.FORWARD - input.BACKWARD, input.RIGHT - input.LEFT, input.mouse_world_pos};
        }
    }
    inline void process_physics(const double& delta) {
        b2World_Step(world_id, delta, PHYSICS_SUBSTEPS_COUNT);
        for (Ship* ship : ships) { ship->physics(delta); }
    }
    inline void draw() {
        spdlog::default_logger()->flush();
        Sprite::predraw(_sprite_resources);
        for (const Sprite* sprite : sprites) { sprite->draw(_sprite_resources, camera.get_view_projection()); }
    }
#ifdef DRAW_DEBUG
    inline void debug_draw() {
        spdlog::default_logger()->flush();
        Sprite::predraw_debug(_sprite_resources);
        for (const Sprite* sprite : sprites) { sprite->draw_debug(_sprite_resources, camera.get_view_projection()); }
    }
#endif

    std::vector<Ship*> ships{};
    // TODO: associating sprites with Texture for using Sprite::predraw() only once per frame
    std::vector<const Sprite*> sprites{};

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
    glfwWindowHintString(GLFW_X11_CLASS_NAME, "FLOATINGMICROSOMA");
    glfwWindowHintString(GLFW_X11_INSTANCE_NAME, "turned");
    GLFWwindow* window = glfwCreateWindow(640, 640, PROJECT_NAME_VERSION, NULL, NULL);
    if (!window) LCRITRET(1, "!window");
    b2WorldDef world_def = b2DefaultWorldDef();
    world_def.gravity = {0.0f, 0.0f};
    world_def.enableContinuous = true;
    world_def.enableContactSoftening = true;
    glfwMakeContextCurrent(window);
    // MangoHud sets its own log level, so we overwrite it
    spdlog::set_level(spdlog::level::trace);

    Game* game = new Game(window, world_def);

    game->ships.push_back(new Ship(game->resource_manager.get_texture("assets/ship01.png"), game->get_world(), Transform({0.0f, 0.0f}, 0.0)));
    game->current_ship = game->ships[0];

    std::array<StaticBody, 4> walls = {
        StaticBody::construct_box_from_texture(game->resource_manager.get_texture("assets/wall02.png"), game->get_world(), Transform({0.0, -(256.0)}, 0.0)),
        StaticBody::construct_box_from_texture(game->resource_manager.get_texture("assets/wall02.png"), game->get_world(), Transform({0.0, (256.0)}, 0.0)),
        StaticBody::construct_box_from_texture(game->resource_manager.get_texture("assets/wall02.png"), game->get_world(),
                                               Transform({-(256.0), 0.0}, glm::radians(90.0))),
        StaticBody::construct_box_from_texture(game->resource_manager.get_texture("assets/wall02.png"), game->get_world(),
                                               Transform({(256.0), 0.0}, glm::radians(90.0)))};
    for (auto&& wall : walls) { game->sprites.push_back(&wall.sprite); }
    game->sprites.push_back(&game->ships[0]->get_sprite());

    {
        int w, h;
        glfwGetWindowSize(window, &w, &h);
        game->_set_viewport_dimensions(w, h);
    }
    glfwSetFramebufferSizeCallback(window, [](GLFWwindow* win, int w, int h) { Game::_get(win)->_set_viewport_dimensions(w, h); });
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    // TODO: 60Hz timer for physics
    glfwSwapInterval(1);
    double last_frame = glfwGetTime(), frame_delta = 0;
    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT);
        game->process_input();
        game->process_physics(1.0 / PHYSICS_RATE);
        game->draw();
#ifdef DRAW_DEBUG
        game->debug_draw();
#endif
        glfwSwapBuffers(window);
        const double now = glfwGetTime();
        frame_delta = now - last_frame;
        last_frame = now;
    }
    glfwDestroyWindow(window);
}
