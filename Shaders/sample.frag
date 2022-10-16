#version 330 core

uniform sampler2D tex0;

in vec2 texCoord;

out vec4 Fragcolor;

void main()
{
	//Fragcolor = vec4(0.0f, 1.0f, 0.0f, 1.0f);

	Fragcolor = texture(tex0, texCoord);
}