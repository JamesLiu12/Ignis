#version 330 core

#ifdef VERTEX_STAGE
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location=3) in vec3 aTangent;
layout (location=4) in vec3 aBitangent;

out VS_OUT {
    vec3 FragPos;
    vec2 TexCoords;
    mat3 TBN;
} vs_out;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    vec3 worldPos = vec3(model * vec4(aPos, 1.0));
    vs_out.FragPos = worldPos;
    vs_out.TexCoords = aTexCoords;

    mat3 M = mat3(model);

    vec3 T = normalize(M * aTangent);
    vec3 B = normalize(M * aBitangent);
    vec3 N = normalize(mat3(transpose(inverse(model))) * aNormal);

    T = normalize(T - dot(T, N) * N);
    B = normalize(B - dot(B, N) * N);

    vs_out.TBN = mat3(T, B, N);

    gl_Position = projection * view * vec4(worldPos, 1.0);
}
#endif

#ifdef FRAGMENT_STAGE
#version 330 core
out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec2 TexCoords;
    mat3 TBN;
} fs_in;

struct Material {
    sampler2D diffuse;
    sampler2D specular;
    sampler2D normal;
    float shininess;
    int hasNormal;
};

struct DirLight {
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform Material material;
uniform DirLight dirLight;
uniform vec3 viewPos;

vec3 GetNormal()
{
    vec3 nTS = texture(material.normal, fs_in.TexCoords).xyz * 2.0 - 1.0;
    return normalize(fs_in.TBN * nTS);
}

void main()
{
    vec3 albedo = texture(material.diffuse, fs_in.TexCoords).rgb;
    vec3 specColor = texture(material.specular, fs_in.TexCoords).rgb;

    vec3 N = (material.hasNormal != 0) ? GetNormal() : normalize(fs_in.TBN[2]);
    vec3 V = normalize(viewPos - fs_in.FragPos);
    vec3 L = normalize(-dirLight.direction);
    vec3 H = normalize(L + V);

    vec3 ambient = dirLight.ambient * albedo;

    float diff = max(dot(N, L), 0.0);
    vec3 diffuse = dirLight.diffuse * diff * albedo;

    float spec = 0.0;
    if (diff > 0.0) {
        spec = pow(max(dot(N, H), 0.0), material.shininess);
    }
    vec3 specular = dirLight.specular * spec * specColor;

    FragColor = vec4(ambient + diffuse + specular, 1.0);
}
#endif