#ifndef VERTEX_H
#define VERTEX_H

#include <glm/glm.hpp>

struct Vertex
{
  glm::vec3 pos;        // Position double x, y, z;
  glm::vec3 normal;     // Normale double nx, ny, nz;
  glm::vec4 color;      // Farben (8-Bit RGB) unsigned char r, g, b;
  float     opacity;    // Opazit�t
  glm::vec3 scale;      // Skalierung
  glm::vec4 rotation;   // Rotation
  float     f_rest[48]; // Platz f�r zus�tzliche Eigenschaften
};

// struct for computeshader sorting position depth with reduced memory consumption
struct VertexPos
{
    glm::vec3 position;
    uint32_t          index;
};


#endif // VERTEX_H
