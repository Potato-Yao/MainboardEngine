$input v_texcoord0

#include <bgfx_shader.sh>

SAMPLER2D(s_tex, 0);
uniform vec4 u_resolution; // x = width, y = height, z = texWidth, w = texHeight

void main()
{
    // Calculate how many tiles fit in the window based on texture size
    // Each tile is the full texture size (e.g., if texture is 32x32, each tile is 32x32 pixels)
    vec2 tileCount = u_resolution.xy / u_resolution.zw;

    // Scale UVs to repeat the texture across the screen
    vec2 tiledUV = v_texcoord0 * tileCount;

    gl_FragColor = texture2D(s_tex, tiledUV);
}