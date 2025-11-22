#pragma once
#include <glm/vec2.hpp>
#include <memory>

#include "mesh.cpp"
#include "resource_manager.cpp"
#include "shader.cpp"
#include "shaders.hpp"
#include "texture.cpp"

class Sprite {
public:
    std::shared_ptr<Texture> _texture;
    std::shared_ptr<Shader> _shader;
    std::shared_ptr<Mesh> _mesh;
    glm::vec2 pos;
    double angle;
    glm::ivec2 dimensions;
    glm::vec2 scale;

    glm::mat4x4 get_model() {
        // create identity matrix
        glm::mat4x4 out{1.0f};
        // set position
        out[3] = glm::vec4(pos, 0.0f, 1.0f);
        // rotate it
        out = glm::rotate(out, float(angle), glm::vec3(0.0f, 0.0f, 1.0f));
        // scale it
        out = glm::scale(out, glm::vec3(glm::vec2(dimensions) * scale, 0.0f));
        return out;
    }

    // TODO: Sprite::predraw() and associating sprites with Texture in Game::sprites
    void draw(const glm::mat4x4& VP) {
        _texture->use(0);
        _shader->use();
        _shader->set_mat4("MVP", VP * get_model());
        _mesh->draw();
    }
    Sprite(const std::shared_ptr<Texture>& texture, ResourceManager& resource_manager, const glm::vec2 scale = glm::vec2(1.0, 1.0),
           const glm::vec2 pos = glm::vec2(0.0, 0.0))
        : _texture(texture),
          dimensions(texture->w(), texture->h()),
          pos(pos),
          scale(scale),
          _shader(resource_manager.get_shader(VERTEX_SHADER_2D, FRAGMENT_SHADER_2D)),
          _mesh(resource_manager.get_mesh_rect(0.5f, 0.5f, 1.0f, 1.0f)) {}
    Sprite(const char* path, ResourceManager& resource_manager, const glm::vec2 scale = glm::vec2(1.0, 1.0), const glm::vec2 pos = glm::vec2(0.0, 0.0))
        : Sprite(resource_manager.get_texture(path), resource_manager, scale, pos) {}
};
