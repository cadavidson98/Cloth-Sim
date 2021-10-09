#version 330

uniform mat4 view_matrix;
uniform mat4 proj_matrix;
uniform mat4 normal_matrix;

layout(location = 0) in vec3 vertex;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 tangent;
layout(location = 3) in vec2 tex_coord;

out vec3 position_in_world_space;
out vec3 normal_in_world_space;
out vec3 tangent_in_world_space;
out mat3 TBN;
out vec2 uv;

void main() {
    uv = tex_coord.xy;
    position_in_world_space = vertex;
    normal_in_world_space = normal;
    tangent_in_world_space = tangent;
    vec3 T = normalize(tangent);
    vec3 N = normalize(normal);
    // Gram-Schmidt re-orthogonalization
    T = normalize(T - dot(T, N) * N);
    vec3 B = cross(N, T);
    TBN = mat3(T, B, N);
    TBN = TBN;
    
    gl_Position = proj_matrix * view_matrix * vec4(vertex,1);
}
