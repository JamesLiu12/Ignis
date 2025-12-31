#version 330 core

// 定义最大光源数量 (根据你的场景需求调整)
#define MAX_DIR_LIGHTS 4
#define MAX_POINT_LIGHTS 16
#define MAX_SPOT_LIGHTS 16

// ----------------------------------------------------------------------------
// VERTEX SHADER
// ----------------------------------------------------------------------------
#ifdef VERTEX_STAGE
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;

out VS_OUT {
    vec3 FragPos;
    vec2 TexCoords;
    mat3 TBN;
} vs_out;

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
    
    // Gram-Schmidt 正交化
    T = normalize(T - dot(T, N) * N);
    // B = cross(N, T); // 如果需要处理镜像纹理

    vs_out.TBN = mat3(T, B, N);

    gl_Position = projection * view * vec4(worldPos, 1.0);
}
#endif

// ----------------------------------------------------------------------------
// FRAGMENT SHADER
// ----------------------------------------------------------------------------
#ifdef FRAGMENT_STAGE
out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec2 TexCoords;
    mat3 TBN;
} fs_in;

struct Material {
    sampler2D albedoMap;
    sampler2D normalMap;
    sampler2D metallicMap;
    sampler2D roughnessMap;
    sampler2D emissiveMap;
    sampler2D aoMap;
};

// --- 光源结构体定义 ---

struct DirectionalLight {
    vec3 direction;
    vec3 radiance;
};

struct PointLight {
    vec3 position;
    vec3 radiance;
    
    // 衰减参数
    float constant;
    float linear;
    float quadratic;
};

struct SpotLight {
    vec3 position;
    vec3 direction;
    vec3 radiance;

    float constant;
    float linear;
    float quadratic;

    float cutOff;      // 内切光角 (cos值)
    float outerCutOff; // 外切光角 (cos值)
};

// --- Uniforms ---

uniform Material material;
uniform vec3 viewPos;

// 光源数组
uniform DirectionalLight directionalLights[MAX_DIR_LIGHTS];
uniform PointLight pointLights[MAX_POINT_LIGHTS];
uniform SpotLight spotLights[MAX_SPOT_LIGHTS];

// 实际激活的光源数量
uniform int numDirectionalLights;
uniform int numPointLights;
uniform int numSpotLights;

const float PI = 3.14159265359;

// ----------------------------------------------------------------------------
// PBR 函数库
// ----------------------------------------------------------------------------

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float num = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float num = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return num / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 getNormalFromMap()
{
    vec3 tangentNormal = texture(material.normalMap, fs_in.TexCoords).xyz * 2.0 - 1.0;
    return normalize(fs_in.TBN * tangentNormal);
}

// ----------------------------------------------------------------------------
// 通用 PBR 计算函数
// ----------------------------------------------------------------------------
// 输入: 光照方向 L, 视线 V, 法线 N, 基础反射率 F0, 材质属性, 光的辐射率 Radiance
// 输出: 该光源贡献的 Lo
vec3 CalcPBRContribution(vec3 L, vec3 V, vec3 N, vec3 F0, vec3 albedo, float roughness, float metallic, vec3 radiance)
{
    vec3 H = normalize(V + L);

    // Cook-Torrance BRDF
    float NDF = DistributionGGX(N, H, roughness);   
    float G   = GeometrySmith(N, V, L, roughness);      
    vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);
       
    vec3 numerator    = NDF * G * F; 
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001; 
    vec3 specular = numerator / denominator;
    
    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;	  

    float NdotL = max(dot(N, L), 0.0);        

    return (kD * albedo / PI + specular) * radiance * NdotL;
}

// ----------------------------------------------------------------------------
// 主函数
// ----------------------------------------------------------------------------
void main()
{
    // 1. 采样纹理 & 准备数据
    vec3 albedo     = pow(texture(material.albedoMap, fs_in.TexCoords).rgb, vec3(2.2));
    vec3 emissive   = pow(texture(material.emissiveMap, fs_in.TexCoords).rgb, vec3(2.2));
    float metallic  = texture(material.metallicMap, fs_in.TexCoords).r;
    float roughness = texture(material.roughnessMap, fs_in.TexCoords).r;
    float ao        = texture(material.aoMap, fs_in.TexCoords).r;

    vec3 N = getNormalFromMap();
    vec3 V = normalize(viewPos - fs_in.FragPos);

    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metallic);

    vec3 Lo = vec3(0.0);

    // ------------------------------------------------------------------------
    // 2. 遍历定向光 (Directional Lights)
    // ------------------------------------------------------------------------
    for(int i = 0; i < numDirectionalLights; i++)
    {
        // 定向光方向是固定的
        vec3 L = normalize(-directionalLights[i].direction);
        vec3 radiance = directionalLights[i].radiance;
        
        Lo += CalcPBRContribution(L, V, N, F0, albedo, roughness, metallic, radiance);
    }

    // ------------------------------------------------------------------------
    // 3. 遍历点光源 (Point Lights)
    // ------------------------------------------------------------------------
    for(int i = 0; i < numPointLights; i++)
    {
        // 计算 L 和 距离
        vec3 L = normalize(pointLights[i].position - fs_in.FragPos);
        float distance = length(pointLights[i].position - fs_in.FragPos);
        
        // 计算衰减
        float attenuation = 1.0 / (pointLights[i].constant + pointLights[i].linear * distance + pointLights[i].quadratic * (distance * distance));
        
        vec3 radiance = pointLights[i].radiance * attenuation;

        Lo += CalcPBRContribution(L, V, N, F0, albedo, roughness, metallic, radiance);
    }

    // ------------------------------------------------------------------------
    // 4. 遍历聚光灯 (Spot Lights)
    // ------------------------------------------------------------------------
    for(int i = 0; i < numSpotLights; i++)
    {
        vec3 L = normalize(spotLights[i].position - fs_in.FragPos);
        float distance = length(spotLights[i].position - fs_in.FragPos);

        // 基础衰减
        float attenuation = 1.0 / (spotLights[i].constant + spotLights[i].linear * distance + spotLights[i].quadratic * (distance * distance));

        // 聚光灯边缘软化
        float theta = dot(L, normalize(-spotLights[i].direction)); 
        float epsilon = max(spotLights[i].cutOff - spotLights[i].outerCutOff, 1e-4);
        float intensity = clamp((theta - spotLights[i].outerCutOff) / epsilon, 0.0, 1.0);

        vec3 radiance = spotLights[i].radiance * attenuation * intensity;

        Lo += CalcPBRContribution(L, V, N, F0, albedo, roughness, metallic, radiance);
    }

    // ------------------------------------------------------------------------
    // 5. 合成最终颜色
    // ------------------------------------------------------------------------
    vec3 ambient = vec3(0.03) * albedo * ao;
    vec3 color = ambient + Lo + emissive;

    // HDR Tone Mapping
    color = color / (color + vec3(1.0));
    // Gamma Correction
    color = pow(color, vec3(1.0/2.2)); 

    FragColor = vec4(color, 1.0);
}
#endif