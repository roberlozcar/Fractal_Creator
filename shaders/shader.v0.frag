#version 430 core

out vec4 outColor;

in vec2 texCoord;

uniform sampler2D color;

void main()
{

	outColor =texture(color,texCoord); 
}
