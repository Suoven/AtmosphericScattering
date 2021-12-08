#version 400 core

layout (location = 0) out vec4 FragColor;

uniform sampler2D gDepth;
uniform sampler2D gNormal;
uniform sampler2D gDiffuse;

struct LightSourceParameters
{
    vec3 color;
    vec3 direction;
    vec3 world_direction;
};

uniform float                       ambient_intensity;
uniform LightSourceParameters       Light;

const float PI = 3.141592653;
const int CascadeMaxCount = 5;
const vec3 debug_colors[5] = vec3[] (vec3(1,0,0), vec3(0,1,0), vec3(0,0,1), vec3(1,1,0), vec3(0,1,1));

uniform sampler2D   shadowMaps[CascadeMaxCount];
uniform float       cascade_planes[CascadeMaxCount];
uniform mat4        lightMtx[CascadeMaxCount];
uniform mat4        ViewToWorld;
uniform mat4        PerspToView;

uniform vec2      size;
uniform int   CascadeCount;
uniform float blend_dist;
uniform float bias;
uniform int   pcf_samples;
uniform bool  draw_cascade_levels;

float ComputePCF(int layer, vec3 worldPos)
{
    //compute the coordinates of the pixel in the shadow map that it belongs to
    vec4 fragPosLightSpace = lightMtx[layer] * vec4(worldPos.xyz, 1.0f);
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;

    //get the depth since it is alreadye in [0,1]
    float depth = projCoords.z;

    //get the coordinates in [0,1]
    projCoords = projCoords * 0.5 + 0.5;

    //check if the pixel is outside the final frustrum
    if (projCoords.z > 1.0) 
        return 1.0;

    float center = (depth + bias) < texture(shadowMaps[layer], projCoords.xy).r ? 1.0 : 0.0f;
    //check if 0 samples
    if(pcf_samples == 0)
        return center;

    // compute pcf for the samples given
    float shadow = 0.0;
    int samples_count = 0;
    vec2 texelSize = 1.0 / vec2(textureSize(shadowMaps[layer], 0));
    for(int x = -pcf_samples; x < pcf_samples; ++x)
    {
        for(int y = -pcf_samples; y < pcf_samples; ++y)
        {
            float pcfDepth = texture(shadowMaps[layer], projCoords.xy + vec2(x, y) * texelSize).r; 
            shadow += (depth + bias) < pcfDepth ? 1.0 : 0.0;  
            samples_count++;
        }    
    }
    //total shadow value
    shadow /= samples_count;
    return shadow;
}

float ComputeShadow(vec3 frag_pos)
{
    // select cascade layer
       vec4 worldfragPos = ViewToWorld * vec4(frag_pos, 1.0);
       float depthValue = abs(frag_pos.z);

       //iterate through the cascade levels
       int layer = -1;
       bool blend_shadow = false;
       for (int i = 0; i < CascadeCount; ++i)
       {
           //check in which layer is the point contained 
           float dist_to_plane = abs(cascade_planes[i] - depthValue);
           if(dist_to_plane < blend_dist)
                blend_shadow = true;
           if (depthValue < cascade_planes[i] || blend_shadow)
           {
               layer = i;
               break;
           }
       }
       //check if point outside the frustrum
       if (layer == -1)
           return 0.0f;

       // get the value of the shadow
       float shadow = ComputePCF(layer, worldfragPos.xyz);

       //check if we need to blend so take the shadow value of the other shadow map
       if(blend_shadow && (layer + 1) < CascadeCount)
       {
            float shadow_to_blend = ComputePCF(layer + 1, worldfragPos.xyz);
            float t = (depthValue - (cascade_planes[layer] - blend_dist)) / (blend_dist + blend_dist);
            shadow = (1 - t) * shadow + t * shadow_to_blend;
       }
       return shadow;
}


vec3 ComputeDebugShadowColor(vec3 frag_pos)
{
       // select cascade layer
       vec4 worldfragPos = ViewToWorld * vec4(frag_pos, 1.0);
       float depthValue = abs(frag_pos.z);

       //iterate through the cascade levels
       int layer = -1;
       bool blend_shadow = false;
       for (int i = 0; i < CascadeCount; ++i)
       {
           //check in which layer is the point contained 
           float dist_to_plane = abs(cascade_planes[i] - depthValue);
           if(dist_to_plane < blend_dist)
                blend_shadow = true;
           if (depthValue < cascade_planes[i] || blend_shadow)
           {
               layer = i;
               break;
           }
       }
       //check if point outside the frustrum
       if (layer == -1)
           return vec3(1,1,1);

       // get the value of the color
       vec3 color = debug_colors[layer];

       //check if we need to blend so take the color value of the other shadow map
       if(blend_shadow && (layer + 1) < CascadeCount)
       {
            vec3 color_to_blend = debug_colors[layer + 1];
            float t = (depthValue - (cascade_planes[layer] - blend_dist)) / (blend_dist + blend_dist);
            color = (1 - t) * color + t * color_to_blend;
       }
       return color;
}

uniform vec3 planet_center;
uniform float planet_radius;
uniform float atm_radius;
uniform float radius_epsilon;
uniform vec3 Br;
uniform vec3 Bme;
uniform vec3 Bms;
uniform float HR;
uniform float HM;
uniform float inscatter_steps;

uniform sampler2D ExtintionTexture;
uniform vec3      camPos;

vec3 GetExtintionInSegment(vec3 top_point, vec3 bottom_point)
{
    vec3 dir = normalize(top_point - bottom_point);
    vec3 normal1 = (top_point - planet_center);
    vec3 normal2 = (bottom_point - planet_center);

    float cos_angle1 = dot(normalize(normal1), dir) * 0.5f + 0.5f;
    float cos_angle2 = dot(normalize(normal2), dir) * 0.5f + 0.5f;

    float h1 = clamp((length(normal1) - planet_radius) / (atm_radius - planet_radius), 0.0f, 1.0f);
    float h2 = clamp((length(normal2) - planet_radius) / (atm_radius - planet_radius), 0.0f, 1.0f);

    vec3 non_extintion = vec3(1.0f);
    vec3 ext1 = texture(ExtintionTexture, vec2(cos_angle1, h1)).rgb;
    vec3 ext2 = texture(ExtintionTexture, vec2(cos_angle2, h2)).rgb;

    return (non_extintion - ext1 + ext2);  
}

vec3 GetExtintionToAtm(float alt, vec3 normal_to_planet)
{
    float h = clamp((alt - planet_radius) / (atm_radius - planet_radius), 0.0f, 1.0f);
    float cos_angle = dot(normal_to_planet, normalize(-Light.world_direction)) * 0.5f + 0.5f;
    return texture(ExtintionTexture, vec2(cos_angle, h)).rgb;
}

bool itersectAtmosphere(in vec3 origin, in vec3 dir, out float mint, out float maxt)
{
    //compute the constants of the quadratic equation
    float b = 2.0f * (dot(dir, origin - planet_center));
    float c = dot(origin - planet_center, origin - planet_center) - (atm_radius * atm_radius - radius_epsilon);

    //precompute some variables of the quadratic equation
    float inside_sqrt = b * b - 4 * c;
    if (inside_sqrt < 0.0f) return false;

    //compute the 2 solutions to the intersection
    float sqrt_result = sqrt(inside_sqrt);
    float t1 = (-b + sqrt_result) / (2.0f);
    float t2 = (-b - sqrt_result) / (2.0f);

     if(t1 < 0.0f && t2 < 0.0f)
        return false;

    //if the origin is inside the sphere we only want the positive time
    if(t1 < 0.0f || t2 < 0.0f)
    {   
        mint = t1 < 0.0f ? t2 : t1;
        return true;
    }

    //return the max or min depending on the input
    maxt = max(t1, t2);
    mint = min(t1, t2);

    return true;
}


bool ComputeEndPoints(in bool is_space, in vec3 world_pos, out vec3 origin, out vec3 dest)
{
    bool point_outside = length(world_pos - planet_center) > atm_radius ? true : false;
    bool cam_outside = length(camPos - planet_center) > atm_radius ? true : false;
    vec3 view = normalize(world_pos - camPos);

    float tmin = 0.0f;
    float tmax = 0.0f;

    if(!cam_outside)
    {
        origin = camPos;
        if(is_space || point_outside)
        {
            itersectAtmosphere(camPos, view, tmin, tmax);
            dest = camPos + view * tmin;
        }
        else
            dest = world_pos;
        return true;
    }
    else
    {
        bool intersection = itersectAtmosphere(camPos, view, tmin, tmax);
        if(!intersection) return false;

        origin = camPos + view * tmin;
        if(is_space || point_outside)
            dest = camPos + view * tmax;
        else
            dest = world_pos;
        return true;
    }
    return true;
}

void GetPhaseFunctions(out float Prigh, out float Pmie, in float cosA)
{
    float g = 0.76f;
    float g2 = g * g;
    float cosA2 = cosA * cosA;
    float inside_sqrt = (1.0f + g2 - 2.0f * g * cosA);
    inside_sqrt = inside_sqrt * inside_sqrt * inside_sqrt;

    Prigh =  (3.0f * (1.0f + cosA2)) / (16.0f * PI);

    Pmie = (3.0f * (1.0f - g2) * (1.0f + cosA2)) / 
            (4.0f * PI * 2.0f * (2.0f + g2) * sqrt(inside_sqrt));
}

vec3 ComputeFinalLight(in vec3 L, in vec3 origin, in vec3 dest)
{
    bool origin_higher = length(origin - planet_center) > length(dest - planet_center) ? true : false;

    vec3 dir = (dest - origin) / inscatter_steps;
    float ds = length(dir);

    vec2 density_CP = vec2(0.0f);
    vec3 Lrigh = vec3(0.0f);
    vec3 Lmie  = vec3(0.0f);

    float Prigh = 0.0f;
    float Pmie = 0.0f;
    float cos_angle = dot(normalize(-Light.world_direction), normalize(dir));
    GetPhaseFunctions(Prigh, Pmie, cos_angle);

    for(float s = 0.5f; s < inscatter_steps; s += 1.0f)
    {
        vec3 p = origin + dir * s;
        vec3 normal_to_planet = p - planet_center;
        float alt = length(p - planet_center);

        vec3 Eap = GetExtintionToAtm(alt, normalize(normal_to_planet));

        vec2 deltaRM = vec2(exp(-(alt - planet_radius) / HR), exp(-(alt - planet_radius) / HM));
        density_CP += deltaRM * ds;

        vec3 Ecp = exp(-(density_CP.x * Br + density_CP.y * Bme));
        vec3 Eapc = Eap * Ecp;

        vec3 dLrigh = deltaRM.x * Br * Eapc * ds * Prigh;
        vec3 dLmie = deltaRM.y * Bms * Eapc * ds * Pmie;

        vec4 view_pos = inverse(ViewToWorld) * vec4(p, 1.0f);

        float v = 1.0f;
        if(ComputeShadow(view_pos.rgb) != 0.0f)
            v = 0.0f;// - ComputeShadow(view_pos.rgb) * 2.0f;

        Lrigh += dLrigh * v;
        Lmie  += dLmie * v;
    }

    vec3 Lin = (Lrigh + Lmie) * Light.color * 20.0f;

    vec3 Extintion = exp(-(density_CP.x * Br + density_CP.y * Bme));

    vec3 ext = vec3(0.0f);
    //vec3 ext2 = 0.0f;
    if(origin_higher)
        ext = GetExtintionInSegment(origin,dest);
    else
        ext = GetExtintionInSegment(dest,origin);

    return Extintion;
}

void main()
{          
    //compute the UV 
    vec2 UV = vec2(gl_FragCoord.x / size.x, gl_FragCoord.y / size.y);
    vec2 XY = UV * 2.0f - 1.0f;
    float Z = (texture(gDepth, UV).r);

    //compute the view position
    vec4 persp_pos = PerspToView * vec4(XY.x, XY.y, Z, 1.0f);
    vec3 view_pos = persp_pos.xyz / persp_pos.w;
    vec3 world_pos = (ViewToWorld * vec4(view_pos, 1.0f)).rgb;

    // retrieve data from gbuffer
    vec3 Normal = texture(gNormal, UV).rgb;   
    vec3 Diffuse = texture(gDiffuse, UV).rgb; 
    float Shininess = texture(gNormal, UV).a; 
    float Specular = texture(gDiffuse, UV).a; 

   ////----------COMPUTE LIGHTING VECTORS---------------//
   vec3 normal     = normalize(Normal);          
   vec3 view       = normalize(-view_pos);
   vec3 light		= normalize(-Light.direction);
   vec3 refl		= reflect(-light, normal);    
   
   //compute extintion to surface from sun
   vec3 normal_planet = world_pos - planet_center;
   vec3 extintion = GetExtintionToAtm(length(normal_planet), normalize(normal_planet));
   vec3 Lsun = extintion * Light.color;
   
   //Lsun =  Light.color;

   //check if the fragment is the space itself
   bool is_space = length(normal_planet) > (atm_radius - radius_epsilon) ? true : false;
   
   //---------COMPUTE LIGTH COMPONENTS----------------//
   vec3 diffuse	 = Lsun * (max(dot(normal, light), 0.0f) * Diffuse);      
   vec3 specular = Lsun * (pow(max(dot(view, refl), 0.0f), 64.0f) * 0.2f); 
	
   //add the resultant color
   vec3 Lground = (diffuse + specular);
   
   //check if we are in debug draw of the cascade levels
   if(draw_cascade_levels)
       Lground *= ComputeDebugShadowColor(view_pos);
   else
       Lground *= (1.0f - ComputeShadow(view_pos));
   
   //compute end points of the ray to ray march for the inscatter light
   vec3 Origin = vec3(0.0f);
   vec3 Dest = vec3(0.0f);
   if(!ComputeEndPoints(is_space, world_pos, Origin, Dest))
        discard;
   
  // Lground = is_space ? Light.color : Lground;
   vec3 result = ComputeFinalLight(Lground, Origin, Dest);
   
   //result = vec3(0.0f);
   //if(length(Dest - PlanetCenter) > length(Origin - PlanetCenter))
    //result = vec3(1.0f);
    
   // Lground = vec3(texture(gDepth, UV).r);
   // result = Lground;
    //set color Normal
    FragColor = vec4(result, 1.0);
}