#pragma once
#include <box2d/math_functions.h>

#include <glm/ext/vector_float2.hpp>
#include <glm/geometric.hpp>

// TODO: union glm::vec2, b2Pos and b2Rot into same class somehow

struct Transform {
    glm::vec2 pos;
    // cos and sin
    b2Rot rot;

    Transform(const Transform& other) : pos(other.pos), rot(other.rot) {}
    Transform(const glm::vec2& pos, const b2Rot& rot) : pos(pos) {
        if (rot.c == 0.0 && rot.s == 0.0)
            this->rot = {0.0, 1.0};
        else {
            glm::vec2 r = glm::normalize(glm::vec2(rot.c, rot.s));
            this->rot = {r.x, r.y};
        }
    }
    Transform(const glm::vec2& pos, const double& angle) : pos(pos), rot{float(std::cos(angle)), float(std::sin(angle))} {}
    Transform(const b2Transform& other) : pos(other.p.x, other.p.y), rot(other.q) {}
};
