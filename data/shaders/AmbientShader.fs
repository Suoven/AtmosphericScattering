#version 400 core

layout (location = 0) out vec4 FragColor;

in vec2 UV;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gDiffuse;
uniform sampler2D gDepthText;
uniform sampler2D AOTexture;

uniform float                       contrast;
uniform float                       ambient_intensity;
uniform int                         texture_mode;
uniform int                         Use_AO;

void main()
{             
    // retrieve data from gbuffer
    vec3 FragPos = texture(gPosition, UV).rgb;
    vec3 Normal = texture(gNormal, UV).rgb;
    vec3 Diffuse = texture(gDiffuse, UV).rgb;
    vec3 Depth = texture(gDepthText, UV).rgb;
    vec3 AO = texture(AOTexture, UV).rgb;

    //	//result vector to store the final color
    vec3 result = Diffuse * ambient_intensity;

    if(texture_mode == 3)
    {result = Diffuse;}

    //ambient occlusion
    if(Use_AO == 1)
        result *= AO;

    //check if we are in a different mode
    if(texture_mode == 1)
     {result = FragPos;}
    if(texture_mode == 2)
    {result = Normal;}
    if(texture_mode == 4)
    {result = (Depth - (1.0f - contrast)) / contrast;}
    if((texture_mode == 5 || texture_mode == 6))
    {result = AO;}
    
    //set the color
    FragColor = vec4(result ,1);
}