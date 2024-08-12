#ifndef VERTEX_H
#define VERTEX_H

#include <glm/glm.hpp>

struct Vertex
{
  glm::vec3 pos;        // position double x, y, z;
  glm::vec3 normal;     // normals double nx, ny, nz;
  glm::vec4 color;      // color (8-Bit RGB) unsigned char r, g, b, a;
  float     opacity;    // opacity
  glm::vec3 scale;      // scale
  glm::vec4 rotation;   // rotatioin quaternion
  float     f_rest[48]; // harmonics values
};

// struct for computeshader sorting position depth with reduced memory consumption
struct VertexPos
{
    glm::vec3 position;
    uint32_t  index;
};

#endif // VERTEX_H