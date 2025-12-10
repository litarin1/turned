#pragma once
#include <map>
#include <memory>
#include <tuple>

#include "mesh.cpp"
#include "shader.cpp"
#include "texture.cpp"

inline constexpr decltype(std::ignore) preload{};

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
        const bool operator<(const ShaderKey& other) const { return vertex_src < other.vertex_src || fragment_src < other.fragment_src; }
    };
    // Rectangle dimensions
    // TODO: use shader uniforms for dimensions instead of different meshes
    //  / actually done by scaling Model matrix (see Sprite::_get_model())
    struct MeshRectKey {
        const float xpivot;
        const float ypivot;
        const float w;
        const float h;

        const bool operator<(const MeshRectKey& other) const { return xpivot < other.xpivot || ypivot < other.ypivot || w < other.w || h < other.h; }
    };

private:
    std::map<TextureKey, std::shared_ptr<Texture> > textures;
    std::map<MeshRectKey, std::shared_ptr<Mesh> > meshes_rect;
    std::map<ShaderKey, std::shared_ptr<Shader> > shaders;

public:
    [[nodiscard("Are you preloading resources? Use preload = get_texture() then")]]
    std::shared_ptr<Texture> get_texture(const char* path) {
        std::shared_ptr<Texture>& ptr = textures[path];
        if (!ptr) ptr = std::make_shared<Texture>(path);
        return ptr;
    }

    [[nodiscard("Are you preloading resources? Use preload = get_shader() then")]]
    std::shared_ptr<Shader> get_shader(const char* vertex_src, const char* fragment_src) {
        auto& ptr = shaders[{vertex_src, fragment_src}];
        if (!ptr) ptr = std::make_shared<Shader>(vertex_src, fragment_src);
        return ptr;
    }

    [[nodiscard("Are you preloading resources? Use preload = get_mesh_rect() then")]]
    std::shared_ptr<Mesh> get_mesh_rect(const float xpivot, const float ypivot, const float w, const float h) {
        const Vertex vertices[] = {
            {{w - xpivot, h - ypivot}, {1.0f, 1.0f}},        // top right
            {{w - xpivot, 0.0f - ypivot}, {1.0f, 0.0f}},     // bottom right
            {{0.0f - xpivot, 0.0f - ypivot}, {0.0f, 0.0f}},  // bottom left
            {{0.0f - xpivot, h - ypivot}, {0.0f, 1.0f}},     // top left
        };
        constexpr uint indices[] = {
            0, 1, 3,  // first triangle
            1, 2, 3   // second triangle
        };
        auto& ptr = meshes_rect[{xpivot, ypivot, w, h}];
        if (!ptr) ptr = std::make_shared<Mesh>(vertices, sizeof(vertices) / sizeof(Vertex), indices, sizeof(indices) / sizeof(uint));
        return ptr;
    }
    std::shared_ptr<Mesh> get_quad_1x1() { return get_mesh_rect(0.5f, 0.5f, 1.0f, 1.0f); }
};
