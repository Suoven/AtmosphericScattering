#version 400 core

layout (location = 0) out vec4 FragColor;

uniform vec3      light_color;

void main()
{      
    //multiply the color so the lights always get applied bloom when enabled
    FragColor = vec4(light_color*20, 1.0);
}