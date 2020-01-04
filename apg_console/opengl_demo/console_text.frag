#version 410

in vec2 v_st;
uniform sampler2D u_tex;
out vec4 o_frag_colour;

void main () {
  vec4 texel = texture( u_tex, v_st );
  o_frag_colour = texture( u_tex, v_st );
}
