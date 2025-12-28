#version 330 core

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

    // 计算 TBN 矩阵 (切线空间 -> 世界空间)
    mat3 M = mat3(model);
    vec3 T = normalize(M * aTangent);
    vec3 B = normalize(M * aBitangent);
    vec3 N = normalize(mat3(transpose(inverse(model))) * aNormal);
    
    // Gram-Schmidt 正交化 (修正 T 和 B 确保垂直)
    T = normalize(T - dot(T, N) * N);
    // 重新计算 B 以处理镜像纹理或左手坐标系模型
    // B = cross(N, T); 

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

// 对应 C++ 的 MaterialData
struct Material {
    sampler2D albedoMap;
    sampler2D normalMap;
    sampler2D metallicMap;
    sampler2D roughnessMap;
    sampler2D emissiveMap;
    sampler2D aoMap;
};

// 定向光结构体
struct DirectionalLight {
    vec3 direction; // 光的方向 (通常是从光源指向场景，例如 vec3(0, -1, 0))
    vec3 color;     // 光的颜色/辐射率 (Radiance)
};

uniform Material material;
uniform DirectionalLight dirLight; // 替换了原来的 Point Light
uniform vec3 viewPos;

const float PI = 3.14159265359;

// ----------------------------------------------------------------------------
// PBR 函数库
// ----------------------------------------------------------------------------

// 法线分布函数 (NDF) - Trowbridge-Reitz GGX
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

// 几何遮蔽函数 (Geometry) - Schlick-GGX
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float num = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return num / denom;
}

// 几何遮蔽函数 (Smith's method)
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

// 菲涅尔方程 (Fresnel) - Fresnel-Schlick
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

// 从法线贴图获取世界空间法线
vec3 getNormalFromMap()
{
    // 从 [0,1] 映射到 [-1,1]
    vec3 tangentNormal = texture(material.normalMap, fs_in.TexCoords).xyz * 2.0 - 1.0;
    // 转换到世界空间
    return normalize(fs_in.TBN * tangentNormal);
}

// ----------------------------------------------------------------------------
// 主函数
// ----------------------------------------------------------------------------
void main()
{
    // 1. 采样纹理
    // Albedo 和 Emissive 通常是 sRGB 空间，需要转换到线性空间进行计算
    vec3 albedo     = pow(texture(material.albedoMap, fs_in.TexCoords).rgb, vec3(2.2));
    vec3 emissive   = pow(texture(material.emissiveMap, fs_in.TexCoords).rgb, vec3(2.2));
    
    // 其他贴图通常已经是线性空间
    float metallic  = texture(material.metallicMap, fs_in.TexCoords).r;
    float roughness = texture(material.roughnessMap, fs_in.TexCoords).r;
    float ao        = texture(material.aoMap, fs_in.TexCoords).r;

    vec3 N = getNormalFromMap();
    vec3 V = normalize(viewPos - fs_in.FragPos);

    // 计算 F0 (基础反射率)
    // 非金属通常为 0.04，金属使用 Albedo 颜色
    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metallic);

    // 反射率方程结果 (Lo)
    vec3 Lo = vec3(0.0);

    // --- 定向光计算 ---
    
    // 1. 计算光照向量 L
    // 对于定向光，L 是常数。
    // 假设 dirLight.direction 是光线照射方向（例如从天顶向下），
    // 我们需要指向光源的方向，所以取反。
    vec3 L = normalize(-dirLight.direction);
    
    // 2. 半程向量 H
    vec3 H = normalize(V + L);
    
    // 3. 辐射率 (Radiance)
    // 定向光没有衰减，直接使用光的颜色
    vec3 radiance = dirLight.color;

    // 4. Cook-Torrance BRDF
    float NDF = DistributionGGX(N, H, roughness);   
    float G   = GeometrySmith(N, V, L, roughness);      
    vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);
       
    vec3 numerator    = NDF * G * F; 
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001; 
    vec3 specular = numerator / denominator;
    
    // kS 是镜面反射部分，等于 Fresnel
    vec3 kS = F;
    // kD 是漫反射部分 (能量守恒: 1.0 - kS)
    vec3 kD = vec3(1.0) - kS;
    // 金属没有漫反射，所以乘以 (1.0 - metallic)
    kD *= 1.0 - metallic;	  

    // NdotL (Lambert 项)
    float NdotL = max(dot(N, L), 0.0);        

    // 累加到 Lo
    Lo += (kD * albedo / PI + specular) * radiance * NdotL; 
    
    // --- 光照计算结束 ---

    // 环境光 (简单的 AO 叠加，实际项目中建议使用 IBL)
    vec3 ambient = vec3(0.03) * albedo * ao;
    
    // 最终颜色 = 环境光 + 累积光照 + 自发光
    vec3 color = ambient + Lo + emissive;

    // HDR 色调映射 (Reinhard)
    color = color / (color + vec3(1.0));

    // Gamma 校正 (转换回 sRGB 用于显示)
    color = pow(color, vec3(1.0/2.2)); 

    FragColor = vec4(color, 1.0);
}
#endif