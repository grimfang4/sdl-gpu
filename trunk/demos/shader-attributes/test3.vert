#ifdef GL_ES
 #version 100
 precision mediump int;
 precision mediump float;
#else
 #version 120
#endif

attribute vec3 gpu_Vertex;
attribute vec2 gpu_TexCoord;
attribute vec4 gpu_Color;
uniform mat4 modelViewProjection;

varying vec4 color;
varying vec2 texCoord;

void main(void)
{
	color = gpu_Color;
	texCoord = vec2(gpu_TexCoord);
	gl_Position = modelViewProjection * vec4(gpu_Vertex, 1.0);
}