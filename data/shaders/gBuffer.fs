#version 400

layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec4 gNormal;
layout (location = 2) out vec4 gDiffuse;

in VS_OUT
{
    vec3 normal;
    vec3 tangent;
    vec3 bitangent;
    vec2 UV;
    vec3 FragPos;
}fs_in;

uniform vec4        DiffuseColor;
uniform sampler2D   NormalTexture;
uniform sampler2D   DiffuseTexture;
uniform sampler2D   SpecularTexture;

uniform bool use_model_normal;
uniform bool use_model_color;
uniform vec3 model_color;

void main()
{    
    //get the data from the texture of the object
    vec3 Diffuse = DiffuseColor.rgb * texture(DiffuseTexture, fs_in.UV).rgb;
    float Specular = texture(SpecularTexture, fs_in.UV).b;
    float Shininess = texture(SpecularTexture, fs_in.UV).g;

    //check if shininess is equal 0
    if(Shininess == 0.0)
    {Shininess = 0.01;}

    //transform the normal
    mat3 TBN = mat3(fs_in.tangent, fs_in.bitangent, fs_in.normal);
    vec3 normal = TBN * (2.0 * texture( NormalTexture, fs_in.UV ).xyz - 1.0);

    if(use_model_normal)
        normal = normalize(fs_in.normal);
    if(use_model_color)
        Diffuse = model_color;

    //fill the gbuffer with information
    gPosition = fs_in.FragPos;
    gNormal   = vec4(normal, Shininess);
    gDiffuse  = vec4(Diffuse, Specular);
}