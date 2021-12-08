#version 400

layout(location = 0) in vec3 vPosition;
layout(location = 1) in vec3 vNormal;
layout(location = 2) in vec4 vTangent;
layout(location = 3) in vec2 vTextCoords;

out VS_OUT
{
    vec3 normal;
    vec3 tangent;
    vec3 bitangent;
    vec2 UV;
    vec3 FragPos;
}vs_out;

uniform mat4 ModelToCamera;
uniform mat4 ModelToPersp;
uniform mat3 NormalMtx;

void main()
{
    //set the position of the vertex with the mvp matrix provided
    gl_Position = ModelToPersp * vec4(vPosition, 1.0f);

    //set the uvs
    vs_out.UV = vTextCoords;

    //compute the frag position in camera space and light space
    vs_out.FragPos = vec3(ModelToCamera * vec4(vPosition, 1.0f));

    //transform the vectors to viewspace
    vs_out.tangent = normalize(vec3(ModelToCamera * vec4(vTangent.xyz, 0)));

   if(vTangent.w > 0.0f)
   {vs_out.bitangent = normalize(vec3(ModelToCamera * vec4(cross(vNormal, vTangent.xyz), 0)));}
   if(vTangent.w < 0.0f)
   {vs_out.bitangent = normalize(vec3(ModelToCamera * vec4(cross(vTangent.xyz, vNormal), 0)));}
       
    vs_out.normal  = normalize(NormalMtx * vNormal);
  
}