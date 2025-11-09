#pragma once
#include <GLFW/glfw3.h>

#include <type_traits>
#include <glm/vec2.hpp>

enum class ActionState : uint8_t {
    RELEASED = 0,
    JUST_PRESSED,
    PRESSED,
    JUST_RELEASED,
};

template <class Derived = void>
struct ActionBase {
    ActionState state{};
    inline static void _press(ActionState& action) {
        if (action == ActionState::RELEASED || action == ActionState::JUST_RELEASED) action = ActionState::JUST_PRESSED;
    }
    inline static void _release(ActionState& action) {
        if (action == ActionState::PRESSED || action == ActionState::JUST_PRESSED) action = ActionState::JUST_RELEASED;
    }
    inline void update(const bool press_or_release, void* user = nullptr) {
        if (press_or_release)
            _press(state);
        else
            _release(state);
        if constexpr (!std::is_void_v<Derived>) static_cast<Derived*>(this)->post_update(user);
    }
};

struct Action : public ActionBase<> {
    inline operator double() const {
        if (state == ActionState::PRESSED || state == ActionState::JUST_PRESSED)
            return 1.0;
        else
            return 0.0;
    }
};
struct AnyCallbackAction : public ActionBase<AnyCallbackAction> {
    void (*callback)(void* userdata, const ActionState state){nullptr};
    void post_update(void* user) { callback(user, state); }
    void operator=(void (*callback)(void* userdata, const ActionState state)) { this->callback = callback; }
};
struct JustPressCallbackAction : public ActionBase<JustPressCallbackAction> {
    void (*callback)(void* userdata){nullptr};
    void post_update(void* user) {
        if (state == ActionState::JUST_PRESSED && callback) callback(user);
    }
    void operator=(void (*callback)(void* userdata)) { this->callback = callback; }
};
#define KEY(key, action)                       \
    case GLFW_KEY_##key:                       \
        action.update(press_or_release, user); \
        break;
#define MOUSEBUTTON(button, action)                       \
    case GLFW_MOUSE_BUTTON_##button:                       \
        action.update(press_or_release, user); \
        break;
struct Input {
public:
    // ACTIONS
    JustPressCallbackAction QUIT;
    JustPressCallbackAction PRINT_HELO;
    Action FORWARD;
    Action BACKWARD;
    Action LEFT;
    Action RIGHT;
    Action TURN_LEFT;
    Action TURN_RIGHT;
    void key_cb(const int key, const bool press_or_release, const int mods, void* user) {
        // TODO: dynamic key mapping
        // TODO: get rid of KEY() macro
        switch (key) {
            // case GLFW_KEY_ESCAPE:
            //     QUIT.update(press_or_release, user);
            //     break;
            // case GLFW_KEY_ENTER:
            //     PRINT_HELO.update(press_or_release, user);
            //     break;
            // case GLFW_KEY_W:
            //     FORWARD.update(press_or_release, user);
            //     break;
            // case GLFW_KEY_S:
            //     BACKWARD.update(press_or_release, user);
            //     break;
            KEY(ESCAPE, QUIT)
            KEY(ENTER, PRINT_HELO)
            KEY(W, FORWARD)
            KEY(S, BACKWARD)
            KEY(A, LEFT)
            KEY(D, RIGHT)
            KEY(Q, TURN_LEFT)
            KEY(E, TURN_RIGHT)
        }
    }
    void mouse_cb(const int button, const bool press_or_release, void* user){
        // TODO: get rid of MOUSEBUTTON() macro
        switch(button){
            MOUSEBUTTON(LEFT, FORWARD)
            MOUSEBUTTON(RIGHT, BACKWARD)
        }
    }
    glm::vec2 mouse_screen_pos{};
    glm::vec2 mouse_world_pos{};
};
