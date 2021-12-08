#version 450 core

layout (location = 0) out vec4 FragColor;

uniform int dir_count;
uniform int step_count;
uniform float angle_bias;
uniform float angle_step;
uniform float radius;
uniform float const_attenuation;
uniform float scale;
uniform bool use_surface_normal;
uniform bool tangents_method;

uniform mat4  ViewToPersp;

in vec2 UV;

uniform sampler2D gPosition;
uniform sampler2D gNormals;

const float PI = 3.141592653;

void main()
{   
    //get the view pos and normal
    vec3 ViewPos = texture(gPosition, UV).rgb;
    vec3 normal = texture(gNormals, UV).rgb;
    
    //get the surface vectors
    vec3 dy = normalize(dFdy(ViewPos));
    vec3 dx = normalize(dFdx(ViewPos));
    
    //get the surface vectors if it is set to do so
    if(use_surface_normal)
        normal = normalize(cross(dx,dy));

    //compute a position placed at the radius in camera space to compute the radius in image space
    vec3 final_pos = ViewPos + vec3(1.0f, 0.0f, 0.0f) * radius;
    vec4 persp_pos = ViewToPersp * vec4(final_pos, 1.0f);
    vec2 newUV = persp_pos.xy / persp_pos.w;
    newUV = newUV * 0.5f + 0.5f;
    
    //get the radius in image space and compute the step size
    float image_radius = length(newUV - UV);
    float step_size = image_radius / float(step_count);
    
    //iterate through all the directions
    float wao = 0.0f, curr_angle = fract(sin( dot(gl_FragCoord.xy, vec2(12.9898, 78.233))) * 43758.5453123);
    for(int i = 0; i < dir_count; i++, curr_angle += angle_step)
    {
        //compute the direction to sample and the tangent and tangent angle
        vec2 dir = normalize(vec2(cos(curr_angle), sin(curr_angle)));
    
        //initialize the max angle
        float max_angle = angle_bias;
    
        //if we want to use the tangent to compute the occlusion
        if(tangents_method)
        {
            vec3 tan = cos(curr_angle) * dx + sin(curr_angle) * dy;
            max_angle = angle_bias + atan(tan.z / length(tan.xy));
        }
    
        //iterate through all of the steps
        for(int j = 1; j <= step_count; j++)
        {
            //compute the horizontal vector and check if it is the same pixel point
            newUV = UV + float(j) * step_size * dir;
            vec3 h_pos = texture(gPosition, newUV).rgb;
            if(h_pos == ViewPos) continue;
    
            //compute horizontal vector and lentgh
            vec3 hor = h_pos - ViewPos;
            float hor_length = length(hor);
            
            //compute the angle between the horizonta vector and the tangent to the plane
            float angle = (PI / 2.0) - acos(dot(normal, normalize(hor)));
            
            //if using tangents method we should get the horizontal method
            if(tangents_method)
                angle = atan(hor.z / length(hor.xy));

            //check if the point is inside the radius and is a greater angle
            if(hor_length < radius && angle > max_angle)
            {
                //compute the attenuation and update the weight ambient occlusion
                float ratio = hor_length / radius;
                float attenuation = 1.0f - (ratio * ratio * const_attenuation);

                //update occlusion as AO(current) - AO(previus) = 
                // = (sin(h_curr) - sin(t)) - (sin(prev) - sin(t)) ->
                // -> AO(curr) - AO(prev) = sin(h_curr) - sin(h_prev)
                float ao = sin(angle) - sin(max_angle);

                //following the formula WAO += W(current) * (AO(current) - AO(prev))
                wao += clamp(ao, 0.0f, 1.0f) * attenuation;
    
                //update the max angle
                max_angle = angle;
            }
        }
    }
    //average occlusion
    wao = clamp((wao * scale) / dir_count, 0.0f, 1.0f);
    
    //result the final occlusion
    vec3 result = vec3(1.0f - wao); 
 
    //set the color
    FragColor = vec4(result,1);
}



