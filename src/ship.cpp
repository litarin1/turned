#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/rotate_vector.hpp>
#include <memory>

#include "input.cpp"
#include "mesh.cpp"
#include "resource_manager.cpp"
#include "shader.cpp"
#include "shaders.hpp"
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
    // px/s2
    double acceleration = 100.0;
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
            vel += (glm::vec2(glm::rotate(float(angle), glm::vec3{0.0f, 0.0f, 1.0f}) * glm::vec4(input_dir * float(dt * acceleration), 0.0f, 0.0f)));
        else if (glm::length(vel) <= acceleration * dt * dt)
            vel = glm::vec2(0.0f, 0.0f);
        else
            vel -= glm::normalize(vel) * float(.4 * dt * dt);
        pos += vel * float(dt);
    }
    Ship(std::shared_ptr<Texture> sprite, glm::vec2 pos = glm::vec2{0.0f, 0.0f}, double rot = 0.0)
        : sprite(sprite), pos(pos), angle(rot), dimensions(sprite->w(), sprite->h()) {}
};
