#version 400
layout (location = 0) in vec3 vPosition;
layout(location = 3)  in vec2 vTextCoords;

out vec2 uvs;

uniform mat4 ModelToPersp;

void main()
{
    //set the position of the vertex with the mvp matrix provided
    gl_Position = ModelToPersp * vec4(vPosition, 1.0f);

    uvs = vTextCoords;
}  