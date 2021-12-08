#version 400 core

layout (location = 0) in vec3 vPos;

uniform mat4 WorldToLightPersp;
uniform mat4 ModelToWorld;

void main()
{
    gl_Position = WorldToLightPersp * ModelToWorld * vec4(vPos, 1.0f);
}