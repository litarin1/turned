#pragma once
#include <GL/gl.h>
#include <sys/types.h>
#include <stb_image.h>

#include "log.cpp"

class Texture {
    uint _id;
    uint _w, _h;

public:
    // only get from ResourceManager
    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;
    Texture(const char* path) {
        static uint count = 0;
        int w, h, nchannels;
        stbi_set_flip_vertically_on_load(true);
        u_char* data = stbi_load(path, &w, &h, &nchannels, 0);
        if (!data)
            LERR("failed to load texture ({}) {}: {}", count, path, stbi_failure_reason());
        else {
            LTRACE("w{} h{} channels{} count{}", w, h, nchannels, count);
            glGenTextures(1, &_id);
            glBindTexture(GL_TEXTURE_2D, _id);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            if (nchannels == 4)
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
            else
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            // glGenerateMipmap(GL_TEXTURE_2D);
        }
        stbi_image_free(data);
        count++;
        _w = w;
        _h = h;
    }

    inline void use(const u_char texture_unit) const {
        glActiveTexture(GL_TEXTURE0 + texture_unit);
        glBindTexture(GL_TEXTURE_2D, _id);
    }
    inline uint w() const {return _w;}
    inline uint h() const {return _h;}
};
