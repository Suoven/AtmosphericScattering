#version 400

layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec4 gNormal;
layout (location = 2) out vec4 gDiffuse;

uniform sampler2D   gDepth_in;

uniform sampler2D   diffuse;
uniform sampler2D   normal;
uniform sampler2D   metallic;

uniform mat4        ModelToWorld;
uniform mat4        WorldToView;
uniform mat4        PerspToView;

uniform vec2        size;
uniform float       min_angle;
uniform int         mode;

const float PI = 3.141592653;

void main()
{    
    //compute the UV 
    vec2 UV = vec2(gl_FragCoord.x / size.x, gl_FragCoord.y / size.y);
    vec2 XY = UV * 2.0f - 1.0f;
    float Z = texture(gDepth_in, UV).r;

    //compute the view position
    vec4 persp_pos = PerspToView * vec4(XY.x, XY.y, Z, 1.0f);
    vec3 view_pos = persp_pos.xyz / persp_pos.w;

    //get the position in the volume cube and perform persp division
    vec4 position = inverse(ModelToWorld) * inverse(WorldToView) * vec4(view_pos, 1.0f);
    vec3 coord = position.xyz / position.w;
    //get the coordinates in [0,1] for the textures
    coord = coord + 0.5;

    //get the information from the textures of the decal
    vec3 decal_mettalic = texture(metallic, coord.xy).rgb;
    vec3 decal_normal   = normalize(texture(normal, coord.xy).rgb * 2.0 - 1.0);
    vec4 decal_diffuse  = texture(diffuse, coord.xy).rgba;
    
    //check if the position is outside of the cube
    if(mode != 1 && (coord.x > 1.0f || coord.x < 0.0f || coord.y > 1.0f || coord.y < 0.0f || coord.z > 1.0f || coord.z < 0.0f || (decal_diffuse.a <= 0.0f && mode != 2)))
         discard;
    else
    {
        //compute the TBN matrix vectors
        vec3 T = -dFdx(view_pos.xyz);
        vec3 B = -dFdy(view_pos.xyz);
        
        //check if we are in the border
        if((T.x == 0.0f && T.y == 0.0f && T.z == 0.0f) || 
          (B.x == 0.0f && B.y == 0.0f && B.z == 0.0f))
            discard;

        //normalize the vectors and compute the normal of the geometry
        T = normalize(T);
        B = normalize(B);
        vec3 N = cross(T.xyz, B.xyz);

        //construc the TBN matric
        mat3 TBN = mat3(T, B, N);
        
        // transform the normal to eye space 
        decal_normal = normalize(decal_normal*TBN);

        //check if the angle between the geometry and de decal is too big so we need to discard rendering onto that geometry
        vec4 cube_normal = normalize(WorldToView * ModelToWorld * vec4(0.0f, 0.0f, -1.0f, 0.0f));
        float angle = acos(dot(N, cube_normal.xyz));

        //check if we are in the second quadrant
        if(angle > PI/2.0f) angle = PI - angle;
        if(mode != 1 && angle > min_angle)
            discard;

        //check if we are in a different render mode
        if(mode != 0)
        {
            //fill the gbuffer with information
            gPosition = view_pos;
            gNormal   = vec4(1.0f, 0.0f, 0.0f, 1.0f);
            gDiffuse  = vec4(vec3(1.0f), 1.0f);
        }
        else
        {
            //fill the gbuffer with information
            gPosition = view_pos;
            gNormal   = vec4(decal_normal, decal_mettalic.g);
            gDiffuse  = vec4(decal_diffuse.xyz, decal_mettalic.b);
        }
    }
}