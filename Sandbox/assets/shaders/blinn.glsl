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

struct PointLight {
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float range;
    float attenuation;
};

struct SpotLight {
    vec3 position;
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float range;
    float attenuation;
    float innerConeAngle;
    float outerConeAngle;
};

uniform Material material;
uniform DirLight dirLight;
uniform PointLight pointLight;
uniform SpotLight spotLight;
uniform vec3 viewPos;
uniform int lightType; // 0 = Directional, 1 = Point, 2 = Spot

vec3 GetNormal()
{
    vec3 nTS = texture(material.normal, fs_in.TexCoords).xyz * 2.0 - 1.0;
    return normalize(fs_in.TBN * nTS);
}

vec3 CalculateDirectionalLight(DirLight light, vec3 normal, vec3 viewDir, vec3 albedo, vec3 specColor)
{
    vec3 L = normalize(-light.direction);
    vec3 H = normalize(L + viewDir);
    
    vec3 ambient = light.ambient * albedo;
    
    float diff = max(dot(normal, L), 0.0);
    vec3 diffuse = light.diffuse * diff * albedo;
    
    float spec = 0.0;
    if (diff > 0.0) {
        spec = pow(max(dot(normal, H), 0.0), material.shininess);
    }
    vec3 specular = light.specular * spec * specColor;
    
    return ambient + diffuse + specular;
}

vec3 CalculatePointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 albedo, vec3 specColor)
{
    vec3 L = normalize(light.position - fragPos);
    vec3 H = normalize(L + viewDir);
    
    // Distance attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0;
    if (distance < light.range) {
        attenuation = 1.0 / (1.0 + light.attenuation * distance * distance);
    } else {
        attenuation = 0.0;
    }
    
    vec3 ambient = light.ambient * albedo * attenuation;
    
    float diff = max(dot(normal, L), 0.0);
    vec3 diffuse = light.diffuse * diff * albedo * attenuation;
    
    float spec = 0.0;
    if (diff > 0.0) {
        spec = pow(max(dot(normal, H), 0.0), material.shininess);
    }
    vec3 specular = light.specular * spec * specColor * attenuation;
    
    return ambient + diffuse + specular;
}

vec3 CalculateSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 albedo, vec3 specColor)
{
    vec3 L = normalize(light.position - fragPos);
    vec3 H = normalize(L + viewDir);
    
    // Distance attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0;
    if (distance < light.range) {
        attenuation = 1.0 / (1.0 + light.attenuation * distance * distance);
    } else {
        attenuation = 0.0;
    }
    
    // Spotlight cone attenuation
    float theta = dot(L, normalize(-light.direction));
    float innerCutoff = cos(radians(light.innerConeAngle));
    float outerCutoff = cos(radians(light.outerConeAngle));
    float epsilon = innerCutoff - outerCutoff;
    float intensity = clamp((theta - outerCutoff) / epsilon, 0.0, 1.0);
    
    attenuation *= intensity;
    
    vec3 ambient = light.ambient * albedo * attenuation;
    
    float diff = max(dot(normal, L), 0.0);
    vec3 diffuse = light.diffuse * diff * albedo * attenuation;
    
    float spec = 0.0;
    if (diff > 0.0) {
        spec = pow(max(dot(normal, H), 0.0), material.shininess);
    }
    vec3 specular = light.specular * spec * specColor * attenuation;
    
    return ambient + diffuse + specular;
}

void main()
{
    vec3 albedo = texture(material.diffuse, fs_in.TexCoords).rgb;
    vec3 specColor = texture(material.specular, fs_in.TexCoords).rgb;

    vec3 N = (material.hasNormal != 0) ? GetNormal() : normalize(fs_in.TBN[2]);
    vec3 V = normalize(viewPos - fs_in.FragPos);
    
    vec3 result;
    if (lightType == 0) {
        // Directional Light
        result = CalculateDirectionalLight(dirLight, N, V, albedo, specColor);
    } else if (lightType == 1) {
        // Point Light
        result = CalculatePointLight(pointLight, N, fs_in.FragPos, V, albedo, specColor);
    } else if (lightType == 2) {
        // Spot Light
        result = CalculateSpotLight(spotLight, N, fs_in.FragPos, V, albedo, specColor);
    } else {
        // Fallback to directional
        result = CalculateDirectionalLight(dirLight, N, V, albedo, specColor);
    }

    FragColor = vec4(result, 1.0);
}
#endif