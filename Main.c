
// MADE BY TSKK-DEV 2025
// I


#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

//-------------------------------------------------------------//
//                      Shader check helper                     //
//-------------------------------------------------------------//
void check_compile_errors(unsigned int shader, const char* type) {
    int success;
    char infoLog[1024];
    if (strcmp(type, "PROGRAM") != 0) {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(shader, 1024, NULL, infoLog);
            printf("ERROR::SHADER_COMPILATION_ERROR of type: %s\n%s\n", type, infoLog);
        }
    }
    else {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(shader, 1024, NULL, infoLog);
            printf("ERROR::PROGRAM_LINKING_ERROR of type: %s\n%s\n", type, infoLog);
        }
    }
}

//-------------------------------------------------------------//
//                         Math structs                         //
//-------------------------------------------------------------//
typedef struct { float x, y, z; } Vec3;

typedef struct {
    unsigned int v_idx[3]; // vertex indices per face tri
    unsigned int n_idx[3]; // normal indices per face tri
} Face;

//-------------------------------------------------------------//
//                    Globals for OBJ data                      //
//-------------------------------------------------------------//
#define MAX_VERTICES 100000
#define MAX_NORMALS  100000
#define MAX_FACES    100000

Vec3 vertices[MAX_VERTICES];
Vec3 normals[MAX_NORMALS];
Face faces[MAX_FACES];

int vertex_count = 0;
int normal_count = 0;
int face_count = 0;

//-------------------------------------------------------------//
//                      OBJ loader function                     //
//-------------------------------------------------------------//
int load_obj(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        printf("FATAL ERROR: Cannot open OBJ file: %s\n", filename);
        return 0;
    }

    char line[128];
    while (fgets(line, sizeof(line), file)) {
        if (strncmp(line, "v ", 2) == 0) {
            sscanf_s(line + 2, "%f %f %f", &vertices[vertex_count].x, &vertices[vertex_count].y, &vertices[vertex_count].z);
            vertex_count++;
        }
        else if (strncmp(line, "vn ", 3) == 0) {
            sscanf_s(line + 3, "%f %f %f", &normals[normal_count].x, &normals[normal_count].y, &normals[normal_count].z);
            normal_count++;
        }
        else if (strncmp(line, "f ", 2) == 0) {
            unsigned int v[3], n[3];
            int matches = sscanf_s(line + 2, "%u//%u %u//%u %u//%u",
                &v[0], &n[0], &v[1], &n[1], &v[2], &n[2]);
            if (matches == 6) {
                for (int i = 0; i < 3; i++) {
                    faces[face_count].v_idx[i] = v[i] - 1; // OBJ is 1-indexed
                    faces[face_count].n_idx[i] = n[i] - 1;
                }
                face_count++;
            }
            else {
                matches = sscanf_s(line + 2, "%u %u %u", &v[0], &v[1], &v[2]);
                if (matches == 3) {
                    for (int i = 0; i < 3; i++) {
                        faces[face_count].v_idx[i] = v[i] - 1;
                        faces[face_count].n_idx[i] = 0;
                    }
                    face_count++;
                }
                else {
                    printf("WARNING: Failed to parse face line: %s", line);
                }
            }
        }
    }
    fclose(file);

    if (normal_count == 0) {
        normals[0].x = 0.0f;
        normals[0].y = 0.0f;
        normals[0].z = 1.0f;
        normal_count = 1;
    }

    printf("OBJ loaded: %d vertices, %d normals, %d faces\n", vertex_count, normal_count, face_count);
    return 1;
}

//-------------------------------------------------------------//
//               Matrix helpers (column-major)                 //
//-------------------------------------------------------------//

void mat4_identity(float* mat) {
    for (int i = 0; i < 16; i++) mat[i] = 0.0f;
    mat[0] = 1.0f;
    mat[5] = 1.0f;
    mat[10] = 1.0f;
    mat[15] = 1.0f;
}

void mat4_perspective(float* mat, float fovy, float aspect, float near, float far) {
    float f = 1.0f / tanf(fovy * 3.14159265f / 360.0f);
    mat[0] = f / aspect;
    mat[1] = 0;
    mat[2] = 0;
    mat[3] = 0;

    mat[4] = 0;
    mat[5] = f;
    mat[6] = 0;
    mat[7] = 0;

    mat[8] = 0;
    mat[9] = 0;
    mat[10] = (far + near) / (near - far);
    mat[11] = -1;

    mat[12] = 0;
    mat[13] = 0;
    mat[14] = (2 * far * near) / (near - far);
    mat[15] = 0;
}

void vec3_sub(Vec3* result, Vec3 a, Vec3 b) {
    result->x = a.x - b.x;
    result->y = a.y - b.y;
    result->z = a.z - b.z;
}

void vec3_normalize(Vec3* v) {
    float len = sqrtf(v->x * v->x + v->y * v->y + v->z * v->z);
    if (len > 0.00001f) {
        v->x /= len;
        v->y /= len;
        v->z /= len;
    }
}

void vec3_cross(Vec3* result, Vec3 a, Vec3 b) {
    result->x = a.y * b.z - a.z * b.y;
    result->y = a.z * b.x - a.x * b.z;
    result->z = a.x * b.y - a.y * b.x;
}

void mat4_lookat(float* mat, Vec3 eye, Vec3 center, Vec3 up) {
    Vec3 f, s, u;
    vec3_sub(&f, center, eye);
    vec3_normalize(&f);

    vec3_cross(&s, f, up);
    vec3_normalize(&s);

    vec3_cross(&u, s, f);

    mat[0] = s.x;
    mat[1] = u.x;
    mat[2] = -f.x;
    mat[3] = 0;

    mat[4] = s.y;
    mat[5] = u.y;
    mat[6] = -f.y;
    mat[7] = 0;

    mat[8] = s.z;
    mat[9] = u.z;
    mat[10] = -f.z;
    mat[11] = 0;

    mat[12] = -(s.x * eye.x + s.y * eye.y + s.z * eye.z);
    mat[13] = -(u.x * eye.x + u.y * eye.y + u.z * eye.z);
    mat[14] = (f.x * eye.x + f.y * eye.y + f.z * eye.z);
    mat[15] = 1;
}

//-------------------------------------------------------------//
//                        Main program                         //
//-------------------------------------------------------------//
int main() {

    if (!glfwInit()) {
        printf("Failed to initialize GLFW\n");
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    //-------------------------------------------------------------//
    //                  Window instance generation                 //
    //-------------------------------------------------------------//

    GLFWwindow* window = glfwCreateWindow(800, 600, "Hello Po bsit 1-1n", NULL, NULL);
    if (window == NULL) {
        printf("FATAL ERROR BOBO KA NU GINAGAWA MO\n");
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    //-------------------------------------------------------------//
    //                     GlAD Initialization                     //
    //-------------------------------------------------------------//

    if (!gladLoadGL()) {
        printf("Failed to initialize GLAD\n");
        return -1;
    }

    glViewport(0, 0, 800, 600);

    //-------------------------------------------------------------//
    //                  Load OBJ and setup buffers                 //
    //-------------------------------------------------------------//

    if (!load_obj("cube.obj")) {  // Make sure cube.obj is in your executable folder
        glfwTerminate();
        return -1;
    }

    float* vertex_data = malloc(sizeof(float) * face_count * 3 * 6); // 3 verts per face, 6 floats per vertex
    if (!vertex_data) {
        printf("Memory allocation failed\n");
        glfwTerminate();
        return -1;
    }

    int idx = 0;
    for (int i = 0; i < face_count; i++) {
        for (int j = 0; j < 3; j++) {
            Vec3 v = vertices[faces[i].v_idx[j]];
            Vec3 n = normals[faces[i].n_idx[j]];
            vertex_data[idx++] = v.x;
            vertex_data[idx++] = v.y;
            vertex_data[idx++] = v.z;
            vertex_data[idx++] = n.x;
            vertex_data[idx++] = n.y;
            vertex_data[idx++] = n.z;
        }
    }

    //-------------------------------------------------------------//
    //                        Shaders                              //
    //-------------------------------------------------------------//

    const char* vertex_shader_source =
        "#version 330 core\n"
        "layout(location = 0) in vec3 aPos;\n"
        "layout(location = 1) in vec3 aNormal;\n"
        "out vec3 Normal;\n"
        "out vec3 FragPos;\n"
        "uniform mat4 model;\n"
        "uniform mat4 view;\n"
        "uniform mat4 projection;\n"
        "void main() {\n"
        "   vec4 worldPos = model * vec4(aPos, 1.0);\n"
        "   FragPos = worldPos.xyz;\n"
        "   Normal = mat3(transpose(inverse(model))) * aNormal;\n"
        "   gl_Position = projection * view * worldPos;\n"
        "}\0";

    const char* fragment_shader_source =
        "#version 330 core\n"
        "in vec3 Normal;\n"
        "in vec3 FragPos;\n"
        "out vec4 FragColor;\n"
        "uniform vec3 viewPos;\n"
        "void main() {\n"
        "   vec3 lightDir = normalize(vec3(1.0, 1.0, 1.0));\n"
        "   vec3 norm = normalize(Normal);\n"
        "   float diff = max(dot(norm, lightDir), 0.0);\n"
        "   vec3 diffuse = diff * vec3(1.0, 0.5, 0.31);\n"
        "   vec3 ambient = vec3(0.1, 0.1, 0.1);\n"
        "   vec3 viewDir = normalize(viewPos - FragPos);\n"
        "   vec3 reflectDir = reflect(-lightDir, norm);\n"
        "   float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);\n"
        "   vec3 specular = spec * vec3(1.0);\n"
        "   vec3 result = ambient + diffuse + specular;\n"
        "   FragColor = vec4(result, 1.0);\n"
        "}\0";

    unsigned int vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_shader_source, NULL);
    glCompileShader(vertex_shader);
    check_compile_errors(vertex_shader, "VERTEX");

    unsigned int fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_shader_source, NULL);
    glCompileShader(fragment_shader);
    check_compile_errors(fragment_shader, "FRAGMENT");

    unsigned int shader_program = glCreateProgram();
    glAttachShader(shader_program, vertex_shader);
    glAttachShader(shader_program, fragment_shader);
    glLinkProgram(shader_program);
    check_compile_errors(shader_program, "PROGRAM");

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    //-------------------------------------------------------------//
    //                     Setup VAO/VBO                            //
    //-------------------------------------------------------------//

    unsigned int VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * face_count * 3 * 6, vertex_data, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    free(vertex_data);

    //-------------------------------------------------------------//
    //                Camera control variables                     //
    //-------------------------------------------------------------//
    float camera_angle = 0.005f;
    float camera_radius = 5.0f;

    //-------------------------------------------------------------//
    //                      Render loop start                       //
    //-------------------------------------------------------------//
    glEnable(GL_DEPTH_TEST);

    float model_matrix[16];
    mat4_identity(model_matrix);

    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.1f, 0.15f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shader_program);

        camera_angle += 0.0005f;

        Vec3 eye = { camera_radius * sinf(camera_angle), 1.5f, camera_radius * cosf(camera_angle) };
        Vec3 center = { 0.0f, 0.0f, 0.0f };
        Vec3 up = { 0.0f, 1.0f, 0.0f };

        float projection[16];
        mat4_perspective(projection, 120.0f, 800.0f / 600.0f, 0.1f, 100.0f);

        float view[16];
        mat4_lookat(view, eye, center, up);

        int model_loc = glGetUniformLocation(shader_program, "model");
        int view_loc = glGetUniformLocation(shader_program, "view");
        int proj_loc = glGetUniformLocation(shader_program, "projection");
        int viewpos_loc = glGetUniformLocation(shader_program, "viewPos");

        glUniformMatrix4fv(model_loc, 1, GL_FALSE, model_matrix);
        glUniformMatrix4fv(view_loc, 1, GL_FALSE, view);
        glUniformMatrix4fv(proj_loc, 1, GL_FALSE, projection);
        glUniform3f(viewpos_loc, eye.x, eye.y, eye.z);

        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, face_count * 3);
        glBindVertexArray(0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    //-------------------------------------------------------------//
    //                         Cleanup                             //
    //-------------------------------------------------------------//
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shader_program);

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
