#version 400 core

out vec4 FragColor;

in vec2 UV;

uniform sampler2D TextureData;
uniform sampler2D PosText;
uniform bool horizontal;
uniform float threshold;

const float weight[5] = float[] (0.2270270270, 0.1945945946, 0.1216216216, 0.0540540541, 0.0162162162);

void main()
{             
    
     //compute texel size and the central pixel brightness value
     vec2 texel = 1.0f / textureSize(TextureData, 0);
     vec3 result = texture(TextureData, UV).rgb * weight[0];
     float center_depth = texture(PosText, UV).b;
     //float threshold = 1.0f;
     float total_weight = weight[0];

     //if horizontal we apply the brightness to the current pixel with the horizontal pixels next to him
     if(horizontal)
         for(int i = 1; i < 5; ++i)
         {
            vec2 UVs = UV + vec2(texel.x * i, 0.0);
            if(abs(center_depth - texture(PosText, UVs).b) < threshold)
            {   
                result += texture(TextureData, UVs).rgb * weight[i];
                total_weight += weight[i];
            }
                

            UVs = UV - vec2(texel.x * i, 0.0);
            if(abs(center_depth - texture(PosText, UVs).b) < threshold)
            {
                result += texture(TextureData, UVs).rgb * weight[i];
                total_weight += weight[i];
            } 
         }

     //if vertical we apply the brightness to the current pixel with the vertical pixels next to him
     else
         for(int i = 1; i < 5; ++i)
         {
            vec2 UVs = UV + vec2(0.0, texel.y * i);
            if(abs(center_depth - texture(PosText, UVs).b) < threshold)
            {
                result += texture(TextureData, UVs).rgb * weight[i];
                total_weight += weight[i];
            }
                

            UVs = UV - vec2(0.0, texel.y * i);
            if(abs(center_depth - texture(PosText, UVs).b) < threshold)
            {
                result += texture(TextureData, UVs).rgb * weight[i];
                total_weight += weight[i];
            }  
         }
        
     result /= total_weight;
     //store the final blurred pixel
     FragColor = vec4(result, 1.0);
}