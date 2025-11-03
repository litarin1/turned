#pragma once
#include <map>

#include "mesh.cpp"
#include "shader.cpp"
#include "texture.cpp"
class ResourceManager {
public:
    // File path
    using TextureKey = const char*;
    // Pointers to shader sources
    // NOT ACTUAL SOURCE DATA, ONLY CONSTANT C-STRINGS
    // TODO: use enums instead of maps for shaders
    struct ShaderKey {
        const char* vertex_src;
        const char* fragment_src;
    };
    // Rectangle dimensions
    // TODO: use shader uniforms for dimensions instead of different meshes
    struct MeshRectKey {
        const float w;
        const float h;
    };

private:
    std::map<TextureKey, std::shared_ptr<Texture> > textures;
    std::map<MeshRectKey, std::shared_ptr<Mesh> > meshes_rect;
    std::map<ShaderKey, std::shared_ptr<Shader> > shaders;

public:
    std::shared_ptr<Texture> get_texture(const char* path) { return textures.try_emplace(path, path).first->second; }
    std::shared_ptr<Shader> get_shader(const char* vertex_src, const char* fragment_src) {
        return shaders.try_emplace({vertex_src, fragment_src}, vertex_src, vertex_src).first->second;
    }
    std::shared_ptr<Mesh> get_mesh_rect(const float w, const float h) { return meshes_rect.try_emplace({w, h}, w, h).first->second; }
};
