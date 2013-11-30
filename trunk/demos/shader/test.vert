#version 120

varying vec4 color;
varying vec2 texCoord;

void main(void)
{
	color = gl_Color;
	texCoord = vec2(gl_MultiTexCoord0);
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
}