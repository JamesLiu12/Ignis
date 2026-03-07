#version 330 core

#ifdef VERTEX_STAGE
layout(location = 0) in vec2 a_Position;
layout(location = 1) in vec2 a_TexCoord;

uniform mat4 u_Projection;

out vec2 v_TexCoord;

void main()
{
    gl_Position = u_Projection * vec4(a_Position, 0.0, 1.0);
    v_TexCoord  = a_TexCoord;
}
#endif

#ifdef FRAGMENT_STAGE
in  vec2 v_TexCoord;
out vec4 FragColor;

uniform sampler2D u_Texture;
uniform vec4      u_Color;
uniform int       u_UseTexture; // 1 = sample texture, 0 = solid color

void main()
{
    vec4 tex  = (u_UseTexture != 0) ? texture(u_Texture, v_TexCoord) : vec4(1.0);
    FragColor = tex * u_Color;
}
#endif