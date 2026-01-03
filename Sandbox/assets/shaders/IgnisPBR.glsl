#version 330 core

// 定义最大光源数量
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

// --- 光源结构体 ---
struct DirectionalLight { vec3 direction; vec3 radiance; };
struct PointLight { vec3 position; vec3 radiance; float constant; float linear; float quadratic; };
struct SpotLight { vec3 position; vec3 direction; vec3 radiance; float constant; float linear; float quadratic; float cutOff; float outerCutOff; };

// --- 环境光设置结构体 (对应 C++ EnvironmentSettings) ---
struct EnvironmentSettings {
    float intensity;
    float rotation; // 弧度制
    vec3 tint;
};

// --- Uniforms ---
uniform Material material;
uniform vec3 viewPos;

// 光源数据
uniform DirectionalLight directionalLights[MAX_DIR_LIGHTS];
uniform PointLight pointLights[MAX_POINT_LIGHTS];
uniform SpotLight spotLights[MAX_SPOT_LIGHTS];
uniform int numDirectionalLights;
uniform int numPointLights;
uniform int numSpotLights;

// --- IBL 贴图 Uniforms ---
// 对应 C++ IBLMaps 和上一轮讨论的 BRDF LUT
uniform samplerCube irradianceMap;  // 漫反射环境图
uniform samplerCube prefilterMap;   // 镜面反射环境图 (带 Mipmap)
uniform sampler2D brdfLUT;          // BRDF 积分查找表
uniform EnvironmentSettings envSettings; // 环境设置

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

// --- 新增: 带粗糙度的 Fresnel ---
// 用于 IBL，防止粗糙表面在边缘出现过强的镜面反射
vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 getNormalFromMap()
{
    vec3 tangentNormal = texture(material.normalMap, fs_in.TexCoords).xyz * 2.0 - 1.0;
    return normalize(fs_in.TBN * tangentNormal);
}

// --- 新增: 环境旋转辅助函数 ---
vec3 rotateVector(vec3 v, float angle)
{
    float s = sin(angle);
    float c = cos(angle);
    // 绕 Y 轴旋转
    mat3 rot = mat3(
        c, 0, s,
        0, 1, 0,
        -s, 0, c
    );
    return rot * v;
}

// 通用 PBR 计算函数 (保持不变)
vec3 CalcPBRContribution(vec3 L, vec3 V, vec3 N, vec3 F0, vec3 albedo, float roughness, float metallic, vec3 radiance)
{
    vec3 H = normalize(V + L);
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
    vec3 R = reflect(-V, N); // 反射向量，用于采样 Prefilter Map

    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metallic);

    vec3 Lo = vec3(0.0);

    // --- 直接光照计算 (Directional, Point, Spot) ---
    // (代码与你原来的一致，省略中间循环以节省篇幅，逻辑不变)
    for(int i = 0; i < numDirectionalLights; i++) {
        vec3 L = normalize(-directionalLights[i].direction);
        Lo += CalcPBRContribution(L, V, N, F0, albedo, roughness, metallic, directionalLights[i].radiance);
    }
    for(int i = 0; i < numPointLights; i++) {
        vec3 L = normalize(pointLights[i].position - fs_in.FragPos);
        float dist = length(pointLights[i].position - fs_in.FragPos);
        float att = 1.0 / (pointLights[i].constant + pointLights[i].linear * dist + pointLights[i].quadratic * (dist * dist));
        Lo += CalcPBRContribution(L, V, N, F0, albedo, roughness, metallic, pointLights[i].radiance * att);
    }
    for(int i = 0; i < numSpotLights; i++) {
        vec3 L = normalize(spotLights[i].position - fs_in.FragPos);
        float dist = length(spotLights[i].position - fs_in.FragPos);
        float att = 1.0 / (spotLights[i].constant + spotLights[i].linear * dist + spotLights[i].quadratic * (dist * dist));
        float theta = dot(L, normalize(-spotLights[i].direction)); 
        float epsilon = max(spotLights[i].cutOff - spotLights[i].outerCutOff, 1e-4);
        float intensity = clamp((theta - spotLights[i].outerCutOff) / epsilon, 0.0, 1.0);
        Lo += CalcPBRContribution(L, V, N, F0, albedo, roughness, metallic, spotLights[i].radiance * att * intensity);
    }

    // ------------------------------------------------------------------------
    // 5. IBL (环境光照) 计算
    // ------------------------------------------------------------------------
    
    // 处理环境旋转 (如果需要)
    vec3 N_rot = rotateVector(N, envSettings.rotation);
    vec3 R_rot = rotateVector(R, envSettings.rotation);

    // A. 环境漫反射 (Diffuse Irradiance)
    // kS 是菲涅尔反射比例，kD 是漫反射比例
    vec3 kS = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);
    vec3 kD = 1.0 - kS;
    kD *= 1.0 - metallic; // 金属没有漫反射
    
    vec3 irradiance = texture(irradianceMap, N_rot).rgb;
    vec3 diffuse    = irradiance * albedo;

    // B. 环境镜面反射 (Specular IBL)
    // 1. 采样 Prefiltered Map (根据粗糙度选择 Mipmap 层级)
    const float MAX_REFLECTION_LOD = 4.0; // 假设你的 Prefilter Map 生成了 5 层 mipmap (0-4)
    vec3 prefilteredColor = textureLod(prefilterMap, R_rot, roughness * MAX_REFLECTION_LOD).rgb;
    
    // 2. 采样 BRDF LUT (Split Sum Approximation 的第二部分)
    vec2 brdf  = texture(brdfLUT, vec2(max(dot(N, V), 0.0), roughness)).rg;
    
    // 3. 组合
    vec3 specular = prefilteredColor * (kS * brdf.x + brdf.y);

    // C. 合成环境光
    vec3 ambient = (kD * diffuse + specular) * ao;
    
    // D. 应用环境设置 (强度和色调)
    ambient *= envSettings.intensity * envSettings.tint;

    // ------------------------------------------------------------------------
    // 6. 最终合成
    // ------------------------------------------------------------------------
    vec3 color = ambient + Lo + emissive;

    // HDR Tone Mapping
    color = color / (color + vec3(1.0));
    // Gamma Correction
    color = pow(color, vec3(1.0/2.2)); 

    FragColor = vec4(color, 1.0);
}
#endif