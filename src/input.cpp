#pragma once
#include <GLFW/glfw3.h>

#include <glm/vec2.hpp>
#include <type_traits>

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

// clang-format off
#define ACTION_FINAL(action) action.update(press_or_release, user); break;
#define ACTION(action) action.update(press_or_release, user);
#define KEY(key) case GLFW_KEY_##key
#define MOUSEBUTTON(button) case GLFW_MOUSE_BUTTON_##button
// clang-format on
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
    // clang-format off
    void key_cb(const int key, const bool press_or_release, const int mods, void* user) {
        // TODO: dynamic key mapping
        switch (key) {
            KEY(ENTER):  ACTION_FINAL(PRINT_HELO)
            KEY(W):      ACTION_FINAL(FORWARD)
            KEY(S):      ACTION_FINAL(BACKWARD)
            KEY(A):      ACTION_FINAL(LEFT)
            KEY(D):      ACTION_FINAL(RIGHT)
            // KEY(Q):      ACTION_FINAL(TURN_LEFT)
            // KEY(E):      ACTION_FINAL(TURN_RIGHT)
            KEY(Q): ACTION(FORWARD) ACTION_FINAL(LEFT)
            KEY(E): ACTION(FORWARD) ACTION_FINAL(RIGHT)
            KEY(ESCAPE): ACTION_FINAL(QUIT)
        }
    }
    void mouse_cb(const int button, const bool press_or_release, void* user) {
        // TODO: get rid of MOUSEBUTTON() macro
        switch (button) {
            MOUSEBUTTON(LEFT) : ACTION_FINAL(FORWARD)
            MOUSEBUTTON(RIGHT) : ACTION_FINAL(BACKWARD)
        }
    }
    // clang-format on
    glm::vec2 mouse_screen_pos{};
    glm::vec2 mouse_world_pos{};
};
