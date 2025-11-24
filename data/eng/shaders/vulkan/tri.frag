#version 450

layout (location = 0) in vec3 fCol;

layout (location = 0) out vec4 oCol;

void main() {
    oCol = vec4(fCol,1);
}
