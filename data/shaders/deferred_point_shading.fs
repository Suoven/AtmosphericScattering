#version 400 core

layout (location = 0) out vec4 FragColor;

in vec2 UV;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gDiffuse;

struct LightSourceParameters
{
    float radius;
    vec3 color;
    vec3 position;
};

uniform float                       ambient_intensity;
uniform LightSourceParameters       Light;
uniform vec2                        size;

const vec3 CamPos = vec3(0,0,0);

void main()
{          
    vec2 UVS = vec2(gl_FragCoord.x / size.x,gl_FragCoord.y / size.y);
    
    // retrieve data from gbuffer
    vec3 FragPos = texture(gPosition, UVS).rgb;
    vec3 Normal = texture(gNormal, UVS).rgb;
    vec3 Diffuse = texture(gDiffuse, UVS).rgb;
    float Shininess = texture(gNormal, UVS).a;
    float Specular = texture(gDiffuse, UVS).a;

    //compute the factors that will affect the lights
    float dist		= distance(FragPos, Light.position);
    float AttFactor = 1.0 - min(dist / Light.radius, 1.0);
       
    ////----------COMPUTE LIGHTING VECTORS-------------------//
    vec3 view       = normalize(CamPos - FragPos);
    vec3 light		= normalize( Light.position - FragPos );
    vec3 refl		= 2*dot(Normal, light) * Normal - light;
    
    //---------COMPUTE LIGTH COMPONENTS----------------//
    vec3 diffuse	= Light.color * (max(dot(Normal, light), 0.0f) * Diffuse);
    vec3 specular	= Light.color * (pow(max(dot(view, refl), 0.0f), Shininess) * Specular); 
	
    //add the resultant color
    vec3 result = AttFactor * (diffuse + specular);

    //set color
    FragColor = vec4(result, 1.0);
}