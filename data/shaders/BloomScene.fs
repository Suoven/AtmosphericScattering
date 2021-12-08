#version 400 core

out vec4 FragColor;

in vec2 UV;

uniform sampler2D HDRTexture;
uniform sampler2D BloomTexture;

uniform bool use_bloom;
uniform bool use_gamma;
uniform bool use_exposure;

uniform float gamma;
uniform float exposure;

void main()
{      
    //get scene color and bloom texture color
    vec3 hdrColor = texture(HDRTexture, UV).rgb;      
    vec3 bloomColor = texture(BloomTexture, UV).rgb;
    
    //apply bloom
    if(use_bloom)
        hdrColor += bloomColor;
    
    //apply exposure to convert the hdr texture to ldr with reinhard exposure
    if(use_exposure)
       hdrColor = 1.0f - exp(-exposure *hdrColor); // (hdrColor * exposure) / ((hdrColor / exposure) + 1);
    
    // apply gamma correction to the image   
    if(use_gamma)
       hdrColor = pow(hdrColor, vec3(1.0 / gamma));
      
     //set the final color   
     FragColor = vec4(hdrColor, 1.0);
}