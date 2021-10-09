#version 330

in vec3 position_in_world_space;
in vec3 normal_in_world_space;
in vec3 tangent_in_world_space;
in vec2 uv;
in mat3 TBN;

out vec4 color;

uniform vec3 light_dir;
uniform vec3 camera_eye;

uniform sampler2D diffuse_map;
uniform sampler2D normal_map;

const float roughness = 1.0;

void main() {
    vec3 to_eye = normalize(camera_eye - position_in_world_space);
    vec3 to_light = normalize(-light_dir);
    
    vec3 normal = texture(normal_map, uv).rgb;
    normal = 2.0 * normal - 1.0;
    normal = TBN * normal;
    normal = normalize(normal);
    //normal = normal_in_world_space;
    float ndotl = max(0, dot(normal, to_light));
    float ndotv = max(0, dot(normal, to_eye));

    float cos_phi = dot(normalize(to_eye - normal * ndotv), normalize(to_light - normal * ndotl));

    float theta_r = acos(ndotl);
    float theta_l = acos(ndotv);
    float alpha = max(theta_l, theta_r);
    float beta = min(theta_l, theta_r);

    float roughness_sqr = roughness * roughness;
    float a = 1 - 0.5 * (roughness_sqr) / (roughness_sqr + 0.33);
    float b = 0.45 * (roughness_sqr) / (roughness_sqr + 0.09);
    vec3 base_clr = texture(diffuse_map, uv).rgb;
    vec3 diffuse = ndotl * (a + b * max(0, cos_phi) * sin(a) * tan(b)) * base_clr;
    color = vec4(diffuse, 1);
}