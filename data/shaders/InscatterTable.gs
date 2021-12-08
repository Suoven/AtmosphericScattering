#version 400 core

layout(triangles, invocations = 5) in;
layout(triangle_strip, max_vertices = 3) out;

void main()
{          
	for (int i = 0; i < 3; ++i)
	{
		gl_Position = gl_in[i].gl_Position;
		gl_Layer = gl_InvocationID;
		EmitVertex();
	}
	EndPrimitive();
}  