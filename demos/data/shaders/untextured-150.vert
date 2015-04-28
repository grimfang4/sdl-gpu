#version 150

in vec3 gpu_Vertex;
in vec4 gpu_Color;
uniform mat4 gpu_ModelViewProjectionMatrix;

out vec4 color;

void main(void)
{
    color = gpu_Color;
    gl_Position = gpu_ModelViewProjectionMatrix * vec4(gpu_Vertex, 1.0);
}