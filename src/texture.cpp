#pragma once
#include <GL/gl.h>
#include <sys/types.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "log.cpp"

class Texture {
    uint _id;

public:
    Texture(const char* path) {
        int w, h, nchannels;
        stbi_set_flip_vertically_on_load(true);
        u_char* data = stbi_load(path, &w, &h, &nchannels, 0);
        if (!data)
            LERR("failed to load texture {}", path);
        else {
            LTRACE("{} {} {}", w, h, nchannels);
            glGenTextures(1, &_id);
            glBindTexture(GL_TEXTURE_2D, _id);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
            // glGenerateMipmap(GL_TEXTURE_2D);
        }
        stbi_image_free(data);
    }

    inline void use(const u_char texture_unit) const {
        glActiveTexture(GL_TEXTURE0 + texture_unit);
        glBindTexture(GL_TEXTURE_2D, _id);
    }
};
