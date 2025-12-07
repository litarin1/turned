#pragma once
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/vector_int2.hpp>
#include <memory>

#include "globals.hpp"
#include "mesh.cpp"
#include "resource_manager.cpp"
#include "shader.cpp"
#include "shaders.hpp"
#include "texture.cpp"
#include "transform.cpp"

class Sprite {
    std::shared_ptr<Texture> _texture;
    glm::ivec2 _dimensions;

public:
    Transform transform;
    glm::vec2 scale;

private:
    glm::mat4x4 _get_model() const {
        glm::mat4x4 out{1.0f};
        // set position
        out[3][0] = transform.pos.x;
        out[3][1] = transform.pos.y;
        out[3][2] = 0.0f;
        // rotate
        out[0][0] = transform.rot.c;
        out[0][1] = -transform.rot.s;
        out[1][0] = transform.rot.s;
        out[1][1] = transform.rot.c;
        // scale
        out[0] *= scale.x * _dimensions.x / ZOOM_FACTOR;
        out[1] *= scale.y * _dimensions.y / ZOOM_FACTOR;
        return out;
    }

public:
    struct _StaticDrawResources {
        std::shared_ptr<Shader> shader;
#ifdef DRAW_DEBUG
        std::shared_ptr<Shader> debug_shader;
#endif
        std::shared_ptr<Mesh> mesh;
        _StaticDrawResources(ResourceManager& manager)
            : shader(manager.get_shader(VERTEX_SHADER_2D, FRAGMENT_SHADER_2D)),
              mesh(manager.get_quad_1x1())
#ifdef DRAW_DEBUG
              ,
              debug_shader(manager.get_shader(VERTEX_SHADER_2D, FRAGMENT_SHADER_2D_SINGLE_COLOR))
#endif
        {
        }
    };

    static void predraw(const _StaticDrawResources& res) {
        res.shader->use();
        res.mesh->use();
    }
    void draw(const _StaticDrawResources& res, const glm::mat4x4 VP) const {
        _texture->use(0);
        res.shader->set_mat4("MVP", VP * _get_model());
        res.mesh->draw();
    }
#ifdef DRAW_DEBUG
    static void predraw_debug(const _StaticDrawResources& res) { res.debug_shader->use(); }
    void draw_debug(const _StaticDrawResources& res, const glm::mat4x4 VP) const {
        _texture->use(0);
        res.shader->set_mat4("MVP", VP * _get_model());
        res.mesh->draw_lines();
    }
#endif

    Sprite(const Sprite&) = delete;
    Sprite& operator=(const Sprite&) = delete;
    Sprite(Sprite&&) = default;
    Sprite& operator=(Sprite&&) = default;
    Sprite(const std::shared_ptr<Texture>& texture, const Transform& transform, const glm::vec2& scale = {1.0f, 1.0f})
        : _texture(texture), transform(transform), scale(scale), _dimensions(texture->w(), texture->h()) {}
};
