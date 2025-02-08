#ifndef MESH_H
#define MESH_H

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"
#include <string>

class Mesh {
public:
    Mesh();
    static Mesh LoadFromFile(std::string file_path);
    void InitGL();
    void Draw();
private:

};
#endif  // MESH_H