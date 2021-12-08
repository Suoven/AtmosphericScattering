#version 400 core

layout (location = 0) out vec4 FragColor;

in vec2 UV;

uniform sampler2D                   HDRTexture;
uniform float                       bright_threshold;

void main()
{      
    //get the color of the pixel
    vec3 hdrColor = texture(HDRTexture, UV).rgb;   

    //check if is a brigth spot
    float bright = dot(hdrColor, vec3(0.2126, 0.7152, 0.0722));
    if(bright > bright_threshold)
        FragColor = vec4(hdrColor, 1.0);
    else
        FragColor = vec4(0.0, 0.0, 0.0, 1.0);
}