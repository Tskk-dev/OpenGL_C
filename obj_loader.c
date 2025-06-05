#define _CRT_SECURE_NO_WARNINGS   // for MSVC to silence sscanf warnings 
#include "obj_loader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    float x, y, z;
} Vec3;

bool load_obj(const char* filename, float** out_vertices, size_t* out_count) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        // Could not open file
        return false;
    }

    // --- First pass: count how many vertex lines and face lines ---
    size_t vertex_line_count = 0;
    size_t face_line_count = 0;
    char buffer[256];

    while (fgets(buffer, sizeof(buffer), file)) {
        if (buffer[0] == 'v' && buffer[1] == ' ') {
            vertex_line_count++;
        }
        else if (buffer[0] == 'f' && buffer[1] == ' ') {
            face_line_count++;
        }
        // ignore everything else
    }

    if (vertex_line_count == 0 || face_line_count == 0) {
        // No vertices or no faces → nothing to load
        fclose(file);
        return false;
    }

    // Allocate arrays of exactly the needed size
    Vec3* temp_vertices = (Vec3*)malloc(vertex_line_count * sizeof(Vec3));
    if (!temp_vertices) {
        fclose(file);
        return false;
    }

    // Each face has exactly 3 indices → total indices = face_line_count * 3
    int* indices = (int*)malloc(face_line_count * 3 * sizeof(int));
    if (!indices) {
        free(temp_vertices);
        fclose(file);
        return false;
    }

    // Rewind file to read again
    fseek(file, 0, SEEK_SET);

    // --- Second pass: actually parse and store ---
    size_t v_idx = 0;  // how many vertices we've read
    size_t f_idx = 0;  // how many indices we've stored so far

    while (fgets(buffer, sizeof(buffer), file)) {
        if (buffer[0] == 'v' && buffer[1] == ' ') {
            // Parse a vertex line: v x y z
            float vx, vy, vz;
            // We expect exactly 3 floats after "v "
            if (sscanf(buffer + 2, "%f %f %f", &vx, &vy, &vz) != 3) {
                // Malformed vertex line
                free(temp_vertices);
                free(indices);
                fclose(file);
                return false;
            }
            temp_vertices[v_idx].x = vx;
            temp_vertices[v_idx].y = vy;
            temp_vertices[v_idx].z = vz;
            v_idx++;
        }
        else if (buffer[0] == 'f' && buffer[1] == ' ') {
            // Parse a face line: f i j k
            // (We only support "f 1 2 3" style—no slashes or extra data.)
            int i1, i2, i3;
            if (sscanf(buffer + 2, "%d %d %d", &i1, &i2, &i3) != 3) {
                // Malformed face line (maybe contains texture/normals or >3 vertices)
                free(temp_vertices);
                free(indices);
                fclose(file);
                return false;
            }
            // Convert from 1-based OBJ index → 0-based C index
            // We also check bounds: each index must lie in [1..vertex_line_count].
            if (i1 < 1 || i1 >(int)vertex_line_count ||
                i2 < 1 || i2 >(int)vertex_line_count ||
                i3 < 1 || i3 >(int)vertex_line_count)
            {
                free(temp_vertices);
                free(indices);
                fclose(file);
                return false;
            }

            indices[f_idx + 0] = i1 - 1;
            indices[f_idx + 1] = i2 - 1;
            indices[f_idx + 2] = i3 - 1;
            f_idx += 3;
        }
        // ignore all other lines (vt, vn, # comments, etc.)
    }

    fclose(file);

    // Now build the final float array: for each face‐index, copy the Vec3 components
    size_t total_indices = face_line_count * 3;       // e.g. face_count*3
    size_t total_floats = total_indices * 3;         // each index → 3 floats
    float* vertex_array = (float*)malloc(total_floats * sizeof(float));
    if (!vertex_array) {
        free(temp_vertices);
        free(indices);
        return false;
    }

    for (size_t i = 0; i < total_indices; ++i) {
        int vi = indices[i];
        vertex_array[i * 3 + 0] = temp_vertices[vi].x;
        vertex_array[i * 3 + 1] = temp_vertices[vi].y;
        vertex_array[i * 3 + 2] = temp_vertices[vi].z;
    }

    // Free temporary buffers
    free(temp_vertices);
    free(indices);

    *out_vertices = vertex_array;
    *out_count = total_floats;
    return true;
}
