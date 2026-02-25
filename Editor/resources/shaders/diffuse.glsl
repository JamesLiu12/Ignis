#version 330 core

#ifdef VERTEX_STAGE
layout (location = 0) in vec3 aPos;
layout (location = 2) in vec2 aTexCoords;

out vec2 TexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    TexCoords = aTexCoords;
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
#endif


#ifdef FRAGMENT_STAGE
out vec4 FragColor;

in vec2 TexCoords;

struct Material {
    sampler2D diffuse;
};

uniform Material material;

void main()
{
    vec3 albedo = texture(material.diffuse, TexCoords).rgb;
    FragColor = vec4(albedo, 1.0);
}
#endif