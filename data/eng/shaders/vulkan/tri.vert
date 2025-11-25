#version 450

// silly vulkan reversey (yes i mean reversey, not reverse y)
vec2 verts[6] = vec2[](
    vec2(0,1), vec2(0,0),
    vec2(1,0), vec2(1,0),
    vec2(1,1), vec2(0,1)
    );

vec3 colors[6] = vec3[](
    vec3(1,0,0),
    vec3(0,1,0),
    vec3(0,0,1),
    vec3(1,0,0),
    vec3(0,1,0),
    vec3(0,0,1)
    );

/*layout (binding = 0) uniform UBO {
    mat4 proj;
    mat4 transf;
    //vec2 pos;
    //vec2 size;
    //vec4 col;
} ubo;*/

layout (location = 0) out vec2 fUV;

void main() {
    vec2 vert = verts[gl_VertexIndex];

    gl_Position = /*ubo.proj * ubo.transf */ vec4(vert, 0,1);
    fUV = vert;
}
