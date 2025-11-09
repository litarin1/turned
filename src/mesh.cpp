#pragma once
#include <sys/types.h>
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>

#include <glm/vec2.hpp>
struct Vertex {
    glm::vec2 pos;
    glm::vec2 texture;
};
class Mesh {
protected:
    uint _VAO, _VBO, _EBO;
    const size_t _nindices;

public:
    // only get from ResourceManager
    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;
    Mesh(const Vertex* vertices, const size_t nvertices, const uint* indices, const size_t nindices) : _nindices(nindices) {
        glGenVertexArrays(1, &_VAO);
        glGenBuffers(1, &_VBO);
        glGenBuffers(1, &_EBO);

        glBindVertexArray(_VAO);
        // fill VBO
        glBindBuffer(GL_ARRAY_BUFFER, _VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * nvertices, vertices, GL_STATIC_DRAW);
        // fill EBO
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint) * nindices, indices, GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, texture)));

        glBindVertexArray(0);
    }

    inline void use() { glBindVertexArray(_VAO); }
    inline void draw() {
        glDrawElements(GL_TRIANGLES, _nindices, GL_UNSIGNED_INT, 0);
    }
    inline void draw_lines(){
        glDrawElements(GL_LINE_LOOP, _nindices, GL_UNSIGNED_INT, 0);
    }
};
