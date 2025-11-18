$input v_texcoord0

#include <bgfx_shader.sh>

SAMPLER2D(s_tex, 0);

void main()
{
    gl_FragColor = texture2D(s_tex, v_texcoord0);
}
$input a_position, a_texcoord0
$output v_texcoord0

#include <bgfx_shader.sh>

void main()
{
    gl_Position = mul(u_modelViewProj, vec4(a_position, 1.0));
    v_texcoord0 = a_texcoord0;
}

