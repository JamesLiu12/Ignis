#version 330 core

#ifdef VERTEX_STAGE
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTexCoord;

out vec2 vTexCoord;

uniform mat4 uViewProjection;

void main()
{
	gl_Position = uViewProjection * vec4(aPos, 1.0);
	vTexCoord = aTexCoord;
}
#endif

#ifdef FRAGMENT_STAGE
out vec4 FragColor;

in vec2 vTexCoord;

uniform sampler2D uTexture;

void main()
{
	FragColor = texture(uTexture, vTexCoord);
}
#endif