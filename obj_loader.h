#ifndef OBJ_LOADER_H
#define OBJ_LOADER_H

#include <stddef.h>
#include <stdbool.h>

// Takes .obj file path and returns flat vertex array
bool load_obj(const char* filename, float** out_vertices, size_t* out_count);

#endif
