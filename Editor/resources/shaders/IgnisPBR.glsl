#version 330 core

#define MAX_DIR_LIGHTS   4
#define MAX_POINT_LIGHTS 16
#define MAX_SPOT_LIGHTS  16

// ============================================================================
// VERTEX SHADER
// ============================================================================
#ifdef VERTEX_STAGE
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;   // UV0
layout (location = 3) in vec2 aTexCoords1;  // UV1
layout (location = 4) in vec2 aTexCoords2;  // UV2
layout (location = 5) in vec3 aTangent;
layout (location = 6) in vec3 aBitangent;

out VS_OUT {
    vec3 FragPos;
    vec2 TexCoords;
    vec2 TexCoords1;
    vec2 TexCoords2;
    mat3 TBN;
} vs_out;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    vec3 worldPos = vec3(model * vec4(aPos, 1.0));
    vs_out.FragPos    = worldPos;
    vs_out.TexCoords  = aTexCoords;
    vs_out.TexCoords1 = aTexCoords1;
    vs_out.TexCoords2 = aTexCoords2;

    mat3 M = mat3(model);
    vec3 T = normalize(M * aTangent);
    vec3 B = normalize(M * aBitangent);
    vec3 N = normalize(mat3(transpose(inverse(model))) * aNormal);

    // Gram-Schmidt 正交化 + 保持手性
    T = normalize(T - dot(T, N) * N);
    float handedness = (dot(cross(N, T), B) < 0.0) ? -1.0 : 1.0;
    B = cross(N, T) * handedness;

    vs_out.TBN = mat3(T, B, N);

    gl_Position = projection * view * vec4(worldPos, 1.0);
}
#endif

// ============================================================================
// FRAGMENT SHADER
// ============================================================================
#ifdef FRAGMENT_STAGE
out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec2 TexCoords;
    vec2 TexCoords1;
    vec2 TexCoords2;
    mat3 TBN;
} fs_in;

struct Material {
    sampler2D albedoMap;
    sampler2D normalMap;
    sampler2D metallicMap;
    sampler2D roughnessMap;
    sampler2D emissiveMap;
    sampler2D aoMap;

    vec4  albedoColor;
    float metallicValue;
    float roughnessValue;
    vec3  emissiveColor;
    float emissiveIntensity;

    sampler2D clearcoatMap;
    sampler2D clearcoatRoughnessMap;
    sampler2D clearcoatNormalMap;
    float clearcoatFactor;
    float clearcoatRoughnessFactor;
};

struct DirectionalLight { vec3 direction; vec3 radiance; };
struct PointLight       { vec3 position;  vec3 radiance; float constant; float linear; float quadratic; };
struct SpotLight        { vec3 position;  vec3 direction; vec3 radiance;
                          float constant; float linear;   float quadratic;
                          float cutOff;   float outerCutOff; };

struct EnvironmentSettings {
    float intensity;
    float rotation;
    vec3  tint;
};

uniform Material material;
uniform vec3     viewPos;

uniform DirectionalLight directionalLights[MAX_DIR_LIGHTS];
uniform PointLight       pointLights[MAX_POINT_LIGHTS];
uniform SpotLight        spotLights[MAX_SPOT_LIGHTS];
uniform int numDirectionalLights;
uniform int numPointLights;
uniform int numSpotLights;

uniform samplerCube      irradianceMap;
uniform samplerCube      prefilterMap;
uniform sampler2D        brdfLUT;
uniform float            prefilterMaxLod;
uniform EnvironmentSettings envSettings;

// ---- 每张纹理使用哪套 UV（默认 0 → 兼容 FBX/OBJ）----
uniform int uv_albedoMap;
uniform int uv_normalMap;
uniform int uv_metallicMap;
uniform int uv_roughnessMap;
uniform int uv_emissiveMap;
uniform int uv_aoMap;
uniform int uv_clearcoatMap;
uniform int uv_clearcoatRoughnessMap;
uniform int uv_clearcoatNormalMap;

// ---- Metallic / Roughness 读取通道（0=R 1=G 2=B 3=A）----
uniform int ch_metallic;
uniform int ch_roughness;

const float PI = 3.14159265359;

// =========================================================================
// 工具函数
// =========================================================================

vec2 selectUV(int idx)
{
    if (idx == 1) return fs_in.TexCoords1;
    if (idx == 2) return fs_in.TexCoords2;
    return fs_in.TexCoords;   // 默认 UV0
}

float selectChannel(vec4 texel, int ch)
{
    if (ch == 1) return texel.g;
    if (ch == 2) return texel.b;
    if (ch == 3) return texel.a;
    return texel.r;           // 默认 R
}

// =========================================================================
// PBR 函数库
// =========================================================================

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a  = roughness * roughness;
    float a2 = a * a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;
    float denom  = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
    return a2 / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;
    return NdotV / (NdotV * (1.0 - k) + k);
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    return GeometrySchlickGGX(NdotV, roughness)
         * GeometrySchlickGGX(NdotL, roughness);
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0)
              * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

float fresnelSchlickScalar(float cosTheta, float f0)
{
    return f0 + (1.0 - f0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 getNormalFromMap(vec2 uv)
{
    vec3 tangentNormal = texture(material.normalMap, uv).xyz * 2.0 - 1.0;
    return normalize(fs_in.TBN * tangentNormal);
}

vec3 getClearcoatNormalFromMap(vec2 uv)
{
    vec3 tangentNormal = texture(material.clearcoatNormalMap, uv).xyz * 2.0 - 1.0;
    return normalize(fs_in.TBN * tangentNormal);
}

vec3 rotateVector(vec3 v, float angle)
{
    float s = sin(angle);
    float c = cos(angle);
    mat3 rot = mat3(
         c, 0, s,
         0, 1, 0,
        -s, 0, c
    );
    return rot * v;
}

// =========================================================================
// Base PBR 直接光 BRDF
// =========================================================================
vec3 CalcPBRContribution(vec3 L, vec3 V, vec3 N, vec3 F0,
                         vec3 albedo, float roughness, float metallic, vec3 radiance)
{
    vec3  H   = normalize(V + L);
    float NDF = DistributionGGX(N, H, roughness);
    float G   = GeometrySmith(N, V, L, roughness);
    vec3  F   = fresnelSchlick(max(dot(H, V), 0.0), F0);

    vec3  numerator   = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
    vec3  specular    = numerator / denominator;

    vec3 kS = F;
    vec3 kD = (vec3(1.0) - kS) * (1.0 - metallic);

    float NdotL = max(dot(N, L), 0.0);
    return (kD * albedo / PI + specular) * radiance * NdotL;
}

// =========================================================================
// Clearcoat 直接光
// =========================================================================
vec2 CalcClearcoatDirect(vec3 L, vec3 V, vec3 Nc, float ccRoughness)
{
    vec3  H     = normalize(V + L);
    float HdotV = max(dot(H, V), 0.0);
    float Fc    = fresnelSchlickScalar(HdotV, 0.04);

    float Dc     = DistributionGGX(Nc, H, ccRoughness);
    float Gc     = GeometrySmith(Nc, V, L, ccRoughness);
    float NcDotV = max(dot(Nc, V), 0.0);
    float NcDotL = max(dot(Nc, L), 0.0);

    float denom = 4.0 * NcDotV * NcDotL + 0.0001;
    float spec  = (Dc * Gc * Fc) / denom * NcDotL;

    return vec2(spec, Fc);
}

// =========================================================================
// 主函数
// =========================================================================
void main()
{
    // ==== 预计算每张纹理的 UV ====
    vec2 uvAlbedo   = selectUV(uv_albedoMap);
    vec2 uvNormal   = selectUV(uv_normalMap);
    vec2 uvMetal    = selectUV(uv_metallicMap);
    vec2 uvRough    = selectUV(uv_roughnessMap);
    vec2 uvEmissive = selectUV(uv_emissiveMap);
    vec2 uvAO       = selectUV(uv_aoMap);
    vec2 uvCC       = selectUV(uv_clearcoatMap);
    vec2 uvCCR      = selectUV(uv_clearcoatRoughnessMap);
    vec2 uvCCN      = selectUV(uv_clearcoatNormalMap);

    // ==== 1. 采样 Base PBR ====
    vec4  albedoSample = texture(material.albedoMap, uvAlbedo);
    vec3  albedo    = pow(albedoSample.rgb, vec3(2.2))
                    * pow(material.albedoColor.rgb, vec3(2.2));
    float alpha     = albedoSample.a * material.albedoColor.a;

    float metallic  = selectChannel(texture(material.metallicMap,  uvMetal), ch_metallic)
                    * material.metallicValue;
    float roughness = selectChannel(texture(material.roughnessMap, uvRough), ch_roughness)
                    * material.roughnessValue;
    float ao        = texture(material.aoMap, uvAO).r;

    vec3  emissive  = pow(texture(material.emissiveMap, uvEmissive).rgb, vec3(2.2))
                    * material.emissiveColor
                    * material.emissiveIntensity;

    // ==== 2. 采样 Clearcoat ====
    float clearcoat          = texture(material.clearcoatMap, uvCC).r
                             * material.clearcoatFactor;
    float clearcoatRoughness = texture(material.clearcoatRoughnessMap, uvCCR).g
                             * material.clearcoatRoughnessFactor;
    clearcoatRoughness = max(clearcoatRoughness, 0.04);

    // ==== 3. 法线 ====
    vec3 N  = getNormalFromMap(uvNormal);
    vec3 Nc = getClearcoatNormalFromMap(uvCCN);
    vec3 V  = normalize(viewPos - fs_in.FragPos);
    vec3 R  = reflect(-V, N);

    vec3 F0 = mix(vec3(0.04), albedo, metallic);

    // ==== 4. 直接光照 ====
    vec3 Lo = vec3(0.0);

    for (int i = 0; i < numDirectionalLights; i++)
    {
        vec3 L = normalize(-directionalLights[i].direction);
        vec3 radiance = directionalLights[i].radiance;
        vec3 baseLo = CalcPBRContribution(L, V, N, F0, albedo, roughness, metallic, radiance);
        vec2 cc     = CalcClearcoatDirect(L, V, Nc, clearcoatRoughness);
        Lo += baseLo * (1.0 - cc.y * clearcoat) + cc.x * clearcoat * radiance;
    }

    for (int i = 0; i < numPointLights; i++)
    {
        vec3  L    = normalize(pointLights[i].position - fs_in.FragPos);
        float dist = length(pointLights[i].position - fs_in.FragPos);
        float att  = 1.0 / (pointLights[i].constant
                          + pointLights[i].linear    * dist
                          + pointLights[i].quadratic * dist * dist);
        vec3 radiance = pointLights[i].radiance * att;
        vec3 baseLo = CalcPBRContribution(L, V, N, F0, albedo, roughness, metallic, radiance);
        vec2 cc     = CalcClearcoatDirect(L, V, Nc, clearcoatRoughness);
        Lo += baseLo * (1.0 - cc.y * clearcoat) + cc.x * clearcoat * radiance;
    }

    for (int i = 0; i < numSpotLights; i++)
    {
        vec3  L         = normalize(spotLights[i].position - fs_in.FragPos);
        float dist      = length(spotLights[i].position - fs_in.FragPos);
        float att       = 1.0 / (spotLights[i].constant
                                + spotLights[i].linear    * dist
                                + spotLights[i].quadratic * dist * dist);
        float theta     = dot(L, normalize(-spotLights[i].direction));
        float epsilon   = max(spotLights[i].cutOff - spotLights[i].outerCutOff, 1e-4);
        float intensity = clamp((theta - spotLights[i].outerCutOff) / epsilon, 0.0, 1.0);
        vec3 radiance   = spotLights[i].radiance * att * intensity;
        vec3 baseLo = CalcPBRContribution(L, V, N, F0, albedo, roughness, metallic, radiance);
        vec2 cc     = CalcClearcoatDirect(L, V, Nc, clearcoatRoughness);
        Lo += baseLo * (1.0 - cc.y * clearcoat) + cc.x * clearcoat * radiance;
    }

    // ==== 5. IBL ====
    vec3 N_rot = rotateVector(N, envSettings.rotation);
    vec3 R_rot = rotateVector(R, envSettings.rotation);

    vec3 kS = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);
    vec3 kD = (vec3(1.0) - kS) * (1.0 - metallic);

    vec3 irradiance       = texture(irradianceMap, N_rot).rgb;
    vec3 diffuse          = irradiance * albedo;

    vec3 prefilteredColor = textureLod(prefilterMap, R_rot, roughness * prefilterMaxLod).rgb;
    vec2 brdf             = texture(brdfLUT, vec2(max(dot(N, V), 0.0), roughness)).rg;
    vec3 baseSpecular     = prefilteredColor * (kS * brdf.x + brdf.y);

    vec3 baseAmbient = (kD * diffuse + baseSpecular) * ao;

    float NcDotV = max(dot(Nc, V), 0.0);
    float Fc_ibl = fresnelSchlickScalar(NcDotV, 0.04);

    vec3 Rc     = reflect(-V, Nc);
    vec3 Rc_rot = rotateVector(Rc, envSettings.rotation);

    vec3 ccPrefilteredColor = textureLod(prefilterMap, Rc_rot, clearcoatRoughness * prefilterMaxLod).rgb;
    vec2 ccBrdf             = texture(brdfLUT, vec2(NcDotV, clearcoatRoughness)).rg;
    vec3 ccSpecularIBL      = ccPrefilteredColor * (0.04 * ccBrdf.x + ccBrdf.y);

    vec3 ambient = baseAmbient * (1.0 - Fc_ibl * clearcoat) + ccSpecularIBL * clearcoat;
    ambient *= envSettings.intensity * envSettings.tint;

    // ==== 6. 最终输出 ====
    vec3 color = ambient + Lo + emissive;
    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0 / 2.2));

    FragColor = vec4(color, alpha);
}
#endif