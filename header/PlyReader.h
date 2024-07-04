#ifndef PLY_READER_H
#define PLY_READER_H

#include <string>
#include <vector>
#include "Vertex.h"

class PlyReader {
public:
    static std::vector<Vertex> readPlyFile(const std::string& filename);
   
};

#endif // PLY_READER_H
