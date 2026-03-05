#version 330 core

#ifdef VERTEX_STAGE
layout(location = 0) in vec2 a_Position;  // 相对锚点的像素偏移
layout(location = 1) in vec2 a_TexCoord;

uniform mat4 u_Model;       // 锚点世界坐标
uniform mat4 u_View;
uniform mat4 u_Projection;
uniform vec2 u_ScreenSize;

out vec2 v_TexCoord;

void main()
{
    v_TexCoord = a_TexCoord;

    // 1. 把锚点投影到裁剪空间
    vec4 clip = u_Projection * u_View * u_Model * vec4(0.0, 0.0, 0.0, 1.0);

    // 2. 像素偏移 → NDC 偏移
    //    乘以 clip.w 是为了抵消后续的透视除法，保证屏幕大小恒定
    vec2 ndc_offset = (a_Position / u_ScreenSize) * 2.0 * clip.w;

    gl_Position = vec4(clip.xy + ndc_offset, clip.z, clip.w);
}
#endif

#ifdef FRAGMENT_STAGE
in  vec2 v_TexCoord;
out vec4 FragColor;

uniform sampler2D u_Atlas;
uniform vec4      u_Color;

void main()
{
    float alpha = texture(u_Atlas, v_TexCoord).r;

    if (alpha < 0.01)
        discard;

    FragColor = vec4(u_Color.rgb, u_Color.a * alpha);
}
#endif