#version 400 core

layout (location = 0) in vec3 vPos;
layout (location = 3) in vec2 vUV;

void main()
{
    gl_Position = vec4(vPos, 1.0f);
}