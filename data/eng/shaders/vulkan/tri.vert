#version 450

// silly vulkan reversey (yes i mean reversey, not reverse y)
vec2 positions[6] = vec2[](
    vec2(-.5,.5), vec2(-.5,-.5),
    vec2(.5,-.5), vec2(.5,-.5),
    vec2(.5,.5), vec2(-.5,.5)
    );

vec3 colors[6] = vec3[](
    vec3(1,0,0),
    vec3(0,1,0),
    vec3(0,0,1),
    vec3(1,0,0),
    vec3(0,1,0),
    vec3(0,0,1)
    );

layout (location = 0) out vec3 fCol;

void main() {
    gl_Position = vec4(positions[gl_VertexIndex], 0,1);
    fCol = colors[gl_VertexIndex];
}
