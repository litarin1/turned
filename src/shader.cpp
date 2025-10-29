#pragma once
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>
#include <sys/types.h>

#include <cstddef>

#include "log.cpp"

class Shader {
    uint id;

public:
    Shader(const char* vertex_src, const char* fragment_src) {
        uint vert, frag;
        int success;
        char logbuf[512];
        // Compile vertex shader
        vert = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vert, 1, &vertex_src, NULL);
        glCompileShader(vert);
        // error check
        glGetShaderiv(vert, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(vert, sizeof(logbuf), NULL, logbuf);
            spdlog::critical("vertex shader compilation failed ({}): {:.{}}", success, logbuf, sizeof(logbuf));
        }

        // Compile fragment shader
        frag = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(frag, 1, &fragment_src, NULL);
        glCompileShader(frag);
        // error check
        glGetShaderiv(frag, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(frag, sizeof(logbuf), NULL, logbuf);
            LCRIT("fragment shader compilation failed ({}): {:.{}}", success, logbuf, sizeof(logbuf));
        }

        // Link to shader program
        id = glCreateProgram();
        glAttachShader(id, vert);
        glAttachShader(id, frag);
        glLinkProgram(id);
        // error check
        glGetProgramiv(id, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(id, sizeof(logbuf), NULL, logbuf);
            LCRIT("shader program linking failed ({}): {:.{}}", success, logbuf, sizeof(logbuf));
        }

        glDeleteShader(vert);
        glDeleteShader(frag);
        if (success != 1)
            LWARN("SHADER NOT COMPILED SUCCESSFULLY ({})", success);
        else
            LTRACE("SHADER COMPILED SUCCESSFULLY ({})", success);
    }
    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;

    inline void use() const { glUseProgram(id); }
};
