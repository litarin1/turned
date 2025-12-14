#include <box2d/box2d.h>
#include <box2d/collision.h>
#include <box2d/id.h>
#include <box2d/math_functions.h>
#include <box2d/types.h>

#include <glm/trigonometric.hpp>

#include "globals.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/rotate_vector.hpp>
#include <memory>

#include "body_factory.cpp"
#include "input.cpp"
#include "sprite.cpp"
#include "texture.cpp"
#include "utils.cpp"

class Ship {
public:
    struct InputFrame {
        // 0.0-1.0
        double throttle = 0.0;
        // 0.0-1.0
        double slide = 0.0;
        // world-space point to turn ship at
        glm::vec2 lookat{};

        void clear() {
            throttle = 0.0;
            slide = 0.0;
            lookat = {0.0, 0.0};
        }
    };
    class IController : public IControllerBase {
    public:
        // called from object's physics()
        virtual Ship::InputFrame get(const Ship& ship) = 0;
    };

    std::shared_ptr<IController> controller{};

private:
    b2BodyId _body_id;
    Sprite _sprite;

    double acceleration{};
    double angular_max_speed{};

public:
    const Transform get_transform() const { return b2Body_GetTransform(_body_id); }
    void set_transform(const Transform& other) { b2Body_SetTransform(_body_id, {other.pos.x, other.pos.y}, other.rot); };

    void physics(const double& dt) {
        InputFrame inputs = controller->get(*this);
        Transform transform = get_transform();
        b2Vec2 vel = b2Body_GetLinearVelocity(_body_id);
        vel.x *= ZOOM_FACTOR;
        vel.y *= ZOOM_FACTOR;

        const b2Rot& q = transform.rot;
        glm::vec2 input = limit_length(glm::vec2(inputs.slide, inputs.throttle), 1.0f) * float(acceleration) * float(dt);
        vel.x += input.x * q.c + input.y * q.s;
        vel.y -= input.x * q.s - input.y * q.c;

        vel.x /= ZOOM_FACTOR;
        vel.y /= ZOOM_FACTOR;

        b2Body_SetLinearVelocity(_body_id, vel);
        glm::vec2 rot = glm::normalize(inputs.lookat - get_transform().pos);
        if (valid_vec2(rot)) b2Body_SetTransform(_body_id, {transform.pos.x, transform.pos.y}, {rot.y, rot.x});

        _sprite.transform = get_transform();
    }

    // gets transform from constructed body
    Ship(const std::shared_ptr<Texture>& texture, b2BodyId&& body, const double& acceleration = 100.0, const double& angular_max_speed = glm::tau<double>())
        : _body_id(body), _sprite(texture, get_transform()), acceleration(acceleration), angular_max_speed(angular_max_speed) {
        b2Body_SetMotionLocks(_body_id, {false, false, true});
    }
    // constructs the body in transform
    Ship(const std::shared_ptr<Texture>& texture, const b2WorldId world, const Transform& transform, const double& acceleration = 100.0,
         const double& angular_max_speed = glm::tau<double>())
        : _body_id(body_factory::circle(world, b2BodyType::b2_dynamicBody, (texture->w() + texture->h()) / 4.0, transform)),
          _sprite(texture, get_transform()),
          acceleration(acceleration),
          angular_max_speed(angular_max_speed) {
        b2Body_SetMotionLocks(_body_id, {false, false, true});
    }

public:
    const Sprite& get_sprite() const { return _sprite; }
};
