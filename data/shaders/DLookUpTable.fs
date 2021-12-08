#version 400 core

layout (location = 0) out vec4 FragColor;

in vec2 UV;

uniform vec2 size;

uniform vec3 planet_center;
uniform float planet_radius;
uniform float atm_radius;
uniform float radius_epsilon;
uniform vec3 Br;
uniform vec3 Bme;
uniform float HR;
uniform float HM;

const int TRANSMITTANCE_INTEGRAL_SAMPLES = 200;

float itersectAtmosphere(in float alt, in float angle)
{
    vec2 orig = vec2(0.0f, alt);
    vec2 dir  = normalize(vec2(1.0f - abs(angle), angle));
    vec2 p_center = vec2(0.0f, planet_center.y);
    vec2 norm = orig - p_center;

    //compute the constants of the quadratic equation
    float b = 2 * (dot(dir, norm));
    float cA = dot(norm, norm) - (atm_radius * atm_radius - radius_epsilon);
    float cE = dot(norm, norm) - (planet_radius * planet_radius);

    //precompute some variables of the quadratic equation
    float inside_sqrtA = b * b - 4 * cA;
    float inside_sqrtE = b * b - 4 * cE;

    //compute the 2 solutions to the intersection
    float sqrtA_result = sqrt(inside_sqrtA);
    float t_atm = (-b + sqrtA_result) / (2.0f);

    float sqrtE_result = sqrt(inside_sqrtE);
    float t1_earth = (-b + sqrtE_result) / (2.0f);
    float t2_earth = (-b - sqrtE_result) / (2.0f);

    if(t1_earth > 0.0f || t2_earth > 0.0f) return -1.0f;
    return t_atm;
}

float densityOverPath(in float scaleHeight, in float alt, in float angle)
{   
    vec2 orig = vec2(0.0f, alt);
    vec2 dir  = normalize(vec2(1.0f - abs(angle), angle));

    float t_Atm = itersectAtmosphere(alt, angle);
    if(t_Atm < 0.0f) return 1e9;

    vec2 intersP = orig + dir * t_Atm;
    float dx = length(intersP - vec2(0.0f, alt)) / float(TRANSMITTANCE_INTEGRAL_SAMPLES);

    float totalDensity = 0.0f;
    float prev_d = exp(-(alt - planet_radius) / scaleHeight);

    for (int i = 1; i <= TRANSMITTANCE_INTEGRAL_SAMPLES; ++i)
    {
        float alt_i = length(orig + dir * float(i) * dx);
        float curr_d = exp(-(alt_i - planet_radius) / scaleHeight);
        totalDensity += ((curr_d + prev_d) / 2.0f) * dx;
    }
    return totalDensity;
}


void main()
{             
   float h = gl_FragCoord.y / size.y;
   float a = (gl_FragCoord.x / size.x) * 2.0f - 1.0f;
   float alt = h * (atm_radius - planet_radius) + planet_radius;
   
   // calculates extinction factor of given altitude and view direction
   vec3 t = Br * densityOverPath(HR, alt, a) + Bme * densityOverPath(HM, alt, a);
   
   //set the color
   FragColor = vec4(exp(-t), 0.0f);
}