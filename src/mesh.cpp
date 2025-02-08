#include "mesh.h"
#include "config.h"

Mesh::Mesh() {

}

/**
 * @brief Load an OBJ and MTL file at the given location
 * 
 * @param file_path relative location to the file
 * @return Mesh
 */
Mesh Mesh::LoadFromFile(std::string file_path) {
    tinyobj::ObjReaderConfig reader_config;
    reader_config.mtl_search_path = "";
    tinyobj::ObjReader file_reader;
    // try debug first
    std::string full_file_path(DEBUG_DIR);
    full_file_path += '/' + file_path;
    if(!file_reader.ParseFromFile(full_file_path, reader_config) || !file_reader.Valid()) {
        
    }
    // file load successful, store the data
    file_reader.
}

void Mesh::InitGL() {

}

void Mesh::Draw() {

}