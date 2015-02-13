#version 120

varying vec4 color;

void main(void)
{
	color.rg = abs(gl_Vertex.xy)*0.002;
	color.b = length(gl_Vertex.xy)*0.002;
    color.a = 1.0;
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
}