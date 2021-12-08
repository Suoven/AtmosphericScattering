#version 400 core

out vec4 FragColor;

in vec2 UV;

uniform sampler2D TextureData;
uniform bool horizontal;

const float weight[5] = float[] (0.2270270270, 0.1945945946, 0.1216216216, 0.0540540541, 0.0162162162);

void main()
{             
     //compute texel size and the central pixel brightness value
     vec2 texel = 1.0f / textureSize(TextureData, 0);
     vec3 result = texture(TextureData, UV).rgb * weight[0];

     //if horizontal we apply the brightness to the current pixel with the horizontal pixels next to him
     if(horizontal)
         for(int i = 1; i < 5; ++i)
         {
            result += texture(TextureData, UV + vec2(texel.x * i, 0.0)).rgb * weight[i];
            result += texture(TextureData, UV - vec2(texel.x * i, 0.0)).rgb * weight[i];
         }

     //if vertical we apply the brightness to the current pixel with the vertical pixels next to him
     else
         for(int i = 1; i < 5; ++i)
         {
             result += texture(TextureData, UV + vec2(0.0, texel.y * i)).rgb * weight[i];
             result += texture(TextureData, UV - vec2(0.0, texel.y * i)).rgb * weight[i];
         }

     //store the final blurred pixel
     FragColor = vec4(result, 1.0);
}