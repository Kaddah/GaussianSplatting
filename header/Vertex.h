#ifndef VERTEX_H
#define VERTEX_H

#include <glm/glm.hpp>

struct Vertex {
    glm::vec3 pos; // Position double x, y, z;
    glm::vec3 normal; // Normale double nx, ny, nz;
    glm::u8vec3 color; // Farben (8-Bit RGB) unsigned char r, g, b;
    
};

#endif // VERTEX_H
