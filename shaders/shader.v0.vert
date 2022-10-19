#version 430 core

layout(location = 0) in vec2 inPos;

uniform mat4 trans;

out vec2 texCoord;

void main()
{
	texCoord = 0.5*inPos + vec2(0.5);
	gl_Position =trans*vec4(inPos,0.,1.);
}
