#version 450

layout (location = 0) in vec2 fUV;

layout (location = 0) out vec4 oCol;

void main() {
    oCol = vec4(fUV, 0,1);
}
