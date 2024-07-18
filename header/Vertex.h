#ifndef VERTEX_H
#define VERTEX_H

#include <glm/glm.hpp>

struct Vertex {
    glm::vec3 pos; // position double x, y, z;
    glm::vec3 normal; // normal double nx, ny, nz;
    glm::vec4 color; // color (8-Bit RGB) unsigned char r, g, b;
    float              opacity;        // opacity for gauss
    glm::vec3          scale;          // scale for gauss
    glm::vec4          rotation;       // rotation for gauss
    float f_rest[48]; // spherical harmonics values 0-2 dc : 3-48 frest 
    
};

// struct for computeshader sorting position depth with reduced memory consumption
struct VertexPos
{
    glm::vec3 position;
    uint32_t          index;
};


#endif // VERTEX_H
