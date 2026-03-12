#version 330 core

// ----------------------------------------------------------------------------
// VERTEX SHADER
// ----------------------------------------------------------------------------
#ifdef VERTEX_STAGE
layout (location = 0) in vec3 aPos;

out vec3 TexCoords;

uniform mat4 projection;
uniform mat4 view;
uniform float envRotation;

mat3 RotationY(float angle)
{
    float c = cos(angle);
    float s = sin(angle);
    return mat3(
         c,  0.0,  s,
        0.0, 1.0, 0.0,
        -s,  0.0,  c
    );
}

void main()
{
    TexCoords = RotationY(envRotation) * aPos;

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

uniform float envIntensity;
uniform vec3  envTint;

void main()
{
    vec3 envColor = texture(environmentMap, TexCoords).rgb;

    envColor *= envTint * envIntensity;

    envColor = envColor / (envColor + vec3(1.0));

    envColor = pow(envColor, vec3(1.0 / 2.2));

    FragColor = vec4(envColor, 1.0);
}
#endif