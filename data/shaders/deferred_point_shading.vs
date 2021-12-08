#version 400 core

layout (location = 0) in vec3 vPos;
layout (location = 3) in vec2 vUV;

out vec2 UV;

uniform mat4 ModelToPersp;

void main()
{
    //set the position of the vertex with the mvp matrix provided
    gl_Position = ModelToPersp * vec4(vPos, 1.0f);

    UV = vUV;
}