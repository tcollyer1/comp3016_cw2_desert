// Each instance of a running fragment shader is responsible for one pixel & its colour

#version 460

layout (location = 0) in vec3 position;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
	gl_Position = projection * view * model * vec4(position.x, position.y, position.z, 1.0);
}