#version 330 core

// ----------------------------------------------------------------------------
// VERTEX SHADER
// ----------------------------------------------------------------------------
#ifdef VERTEX_STAGE
layout (location = 0) in vec3 aPos;

out vec3 TexCoords;

uniform mat4 projection;
uniform mat4 view;

void main()
{
    TexCoords = aPos;
    vec4 pos = projection * mat4(mat3(view)) * vec4(aPos, 1.0);
    
    gl_Position = pos.xyww; 
}
#endif

// ----------------------------------------------------------------------------
// FRAGMENT SHADER
// ----------------------------------------------------------------------------
#ifdef FRAGMENT_STAGE
out vec4 FragColor;

in vec3 TexCoords;

uniform samplerCube environmentMap; 

void main()
{    
    vec3 envColor = texture(environmentMap, TexCoords).rgb;
    
    FragColor = vec4(envColor, 1.0);
}
#endif