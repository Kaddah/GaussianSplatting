#ifndef VERTEX_H
#define VERTEX_H

#include <glm/glm.hpp>

struct Vertex {
    glm::vec3 pos; // Position double x, y, z;
    glm::vec3 normal; // Normale double nx, ny, nz;
    glm::vec4 color; // Farben (8-Bit RGB) unsigned char r, g, b;
    float              opacity;        // Opazität
    glm::vec3          scale;          // Skalierung
    glm::vec4          rotation;       // Rotation
    float f_rest[48]; // Platz für zusätzliche Eigenschaften
    
};

#endif // VERTEX_H
