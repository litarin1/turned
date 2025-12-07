#pragma once

#include <box2d/box2d.h>
#include <box2d/collision.h>
#include <box2d/id.h>
#include <box2d/types.h>

#include <memory>

#include "body_factory.cpp"
#include "sprite.cpp"
class StaticBody {
    b2BodyId _body_id;

public:
    Sprite sprite;

    const Transform get_transform() const { return b2Body_GetTransform(_body_id); }
    void set_transform(const Transform& other) { b2Body_SetTransform(_body_id, {other.pos.x, other.pos.y}, other.rot); };

    StaticBody(const std::shared_ptr<Texture>& texture, b2BodyId&& body_id) : _body_id(body_id), sprite(texture, get_transform()) {}

    static StaticBody construct_box_from_texture(const std::shared_ptr<Texture>& texture, b2WorldId world_id, const Transform& transform) {
        return StaticBody(texture, body_factory::box(world_id, b2BodyType::b2_staticBody, texture->w(), texture->h(), transform));
    }
};
