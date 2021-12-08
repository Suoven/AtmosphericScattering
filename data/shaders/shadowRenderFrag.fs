#version 400

out vec3 FragColor;
  
in vec2 uvs;

uniform sampler2D shadowMap;

void main()
{             
    float depth = texture(shadowMap, uvs).r;
    FragColor = vec3(depth); 
}  