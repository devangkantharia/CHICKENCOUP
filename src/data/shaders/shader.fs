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
  vec3 sun_pos = vec3(0.0, 10.0, 5.0);
  
  vec3 light_dir = normalize(sun_pos - frag);
  vec3 norm = normalize(normals);
  
  float diffuse = max(dot(norm, light_dir), 0.0);
  color = texture(u_texture, uv) * diffuse;
}
