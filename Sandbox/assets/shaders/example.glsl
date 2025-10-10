#version 330 core

#ifdef VERTEX_STAGE	
layout(location = 0) in vec3 aPos;

uniform mat4 uViewProjection;

void main()
{
	gl_Position = uViewProjection * vec4(aPos, 1.0);
}
#endif

#ifdef FRAGMENT_STAGE		
out vec4 FragColor;
void main()
{
	FragColor = vec4(0.8, 0.2, 0.3, 1.0);
}
#endif