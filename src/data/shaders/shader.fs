#version 300 es 

precision mediump float;

in vec2 uv;
in vec3 frag;
in vec3 normals;
in vec4 frag_light_pos;

out vec4 color;

uniform vec3 u_view_position;
uniform sampler2D u_texture;

/* dir light */
struct dir_light {
  vec3 position;
  vec3 target;
  vec3 color;
  float far;
};
uniform dir_light u_dir_light;
uniform sampler2D u_dir_depth;
uniform bool u_dir_active;
/* ------------ */

vec3 calc_dir_light(dir_light l, sampler2D depth)
{
  vec3 proj = frag_light_pos.xyz / frag_light_pos.w;
  proj = proj * 0.5 + 0.5;
  vec3 norm       = normalize(normals);

  // diffuse
  vec3 light_dir  = normalize(l.position - l.target);
  vec3 diff       = texture(u_texture, uv).rgb;
  vec3 diffuse    = max(dot(light_dir, norm), 0.0) * diff * l.color;

  // shadow biasing
  float costheta = clamp(dot(norm, light_dir), 0.0, 1.0);
  float bias     = 0.5*tan(acos(costheta));
  bias           = clamp(bias, 0.01, 0.5);
  
  // shadows
  vec3 frag_to_light  = l.target - l.position;
  float current_depth = proj.z * l.far;
  vec2 tsize = 1.0 / vec2(textureSize(u_dir_depth, 0));
  float shadow = 0.0;
  for (int x=-1; x<=1; ++x) {
    for (int y=-1; y<=1; ++y) {
      float pcf_depth = texture(u_dir_depth, proj.xy + vec2(x, y) * tsize).r;
      pcf_depth *= l.far;
      shadow += current_depth - bias > pcf_depth ? 1.0 : 0.0;
    }
  }
  shadow /= 9.0;

  if (proj.z >= 1.0)
    shadow = 0.0f;

  return vec3((1.0 - shadow) * diffuse);
}

void main()
{
  color = vec4(0.0);
  vec4 diff = texture(u_texture, uv) * 0.5f;

  if (diff.a < 0.5f)
    discard; 

  if (u_dir_active)
    color += diff + vec4(calc_dir_light(u_dir_light, u_dir_depth), 1.0f);
}
