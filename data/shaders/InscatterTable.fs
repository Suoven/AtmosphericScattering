#version 400 core

out vec4 FragColor;

uniform sampler2D Transmittance;

void main()
{             
     
     //store the final blurred pixel
     FragColor = vec4(vec3(1.0f, 0.0f, 0.0f), 1.0);
}