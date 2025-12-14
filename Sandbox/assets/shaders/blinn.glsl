#version 330 core

#ifdef VERTEX_STAGE
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    FragPos   = vec3(model * vec4(aPos, 1.0));
    Normal    = mat3(transpose(inverse(model))) * aNormal;
    TexCoords = aTexCoords;

    gl_Position = projection * view * vec4(FragPos, 1.0);
}
#endif

#ifdef FRAGMENT_STAGE
out vec4 FragColor;

struct Material {
    sampler2D diffuse;
    sampler2D specular;
    float shininess;
};

struct DirLight {
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

uniform DirLight dirLight;
uniform Material material;

uniform vec3 viewPos;

vec3 CalcDirLightBlinn(DirLight light, vec3 normal, vec3 viewDir)
{
    vec3 lightDir = normalize(-light.direction);
    vec3 halfDir  = normalize(lightDir + viewDir);

    vec3 albedo    = texture(material.diffuse,  TexCoords).rgb;
    vec3 specColor = texture(material.specular, TexCoords).rgb;

    // Ambient
    vec3 ambient = light.ambient * albedo;

    // Diffuse (Lambert)
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * albedo;

    // Specular (Blinn-Phong)
    float spec = 0.0;
    if (diff > 0.0)
    {
        float nh = max(dot(normal, halfDir), 0.0);
        spec = pow(nh, material.shininess);
    }
    vec3 specular = light.specular * spec * specColor;

    return ambient + diffuse + specular;
}

void main()
{
    vec3 N = normalize(Normal);
    vec3 V = normalize(viewPos - FragPos);

    vec3 color = CalcDirLightBlinn(dirLight, N, V);
    FragColor = vec4(color, 1.0);
}
#endif