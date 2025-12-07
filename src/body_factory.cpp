#pragma once

#include <box2d/box2d.h>
#include <box2d/collision.h>
#include <box2d/id.h>
#include <box2d/types.h>

#include "globals.hpp"
#include "transform.cpp"

namespace body_factory {
    b2BodyId _body(const b2WorldId& world_id, const b2BodyType& type, Transform transform_unscaled) {
        // scale it
        transform_unscaled.pos /= float(ZOOM_FACTOR);
        b2BodyDef def = b2DefaultBodyDef();
        def.position = {transform_unscaled.pos.x, transform_unscaled.pos.y};
        def.rotation = transform_unscaled.rot;
        def.type = type;
        return b2CreateBody(world_id, &def);
    }
    b2ShapeDef _default_shapedef() {
        b2ShapeDef out = b2DefaultShapeDef();
        out.material.friction = 0.0;
        return out;
    }

    b2BodyId box(const b2WorldId& world_id, const b2BodyType& type, const double& w, const double& h, Transform transform_unscaled) {
        b2BodyId body_id = _body(world_id, type, transform_unscaled);
        b2ShapeDef shape_def = _default_shapedef();
        b2Polygon shape = b2MakeBox(w / ZOOM_FACTOR / 2.0, h / ZOOM_FACTOR / 2.0);
        b2CreatePolygonShape(body_id, &shape_def, &shape);
        return body_id;
    }
    b2BodyId circle(const b2WorldId& world_id, const b2BodyType& type, const double radius, Transform transform_unscaled) {
        b2BodyId body_id = _body(world_id, type, transform_unscaled);
        b2ShapeDef shape_def = _default_shapedef();
        b2Circle shape{{0.0, 0.0}, float(radius / ZOOM_FACTOR)};
        b2CreateCircleShape(body_id, &shape_def, &shape);
        return body_id;
    }
};  // namespace body_factory
