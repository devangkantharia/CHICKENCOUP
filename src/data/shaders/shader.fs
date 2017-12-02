#version 300 es

precision mediump float;

in vec2 uv;
in vec3 frag;
in vec3 normals;

out vec4 color;

uniform vec3 u_view_position;
uniform sampler2D u_texture;

void main()
{
  color = texture(u_texture, uv) * 0.2f;
}
