#version 400 core

layout (location = 0) in vec3 vPos;
layout (location = 3) in vec2 vUV;

out vec2 UV;

void main()
{
    gl_Position = vec4(vPos, 1.0f);
    UV = vUV;
}