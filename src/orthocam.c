#include "orthocam.h"
#include <stdlib.h>

ex_ortho_camera_t* ex_ortho_camera_new(float x, float y, float z, float left, float right, float bottom, float top)
{
  ex_ortho_camera_t *c = malloc(sizeof(ex_ortho_camera_t));

  c->position[0] = 0.0f+x;
  c->position[1] = 0.0f+y;
  c->position[2] = 0.0f+z;
  
  c->front[0] = 0.0f;
  c->front[1] = 0.0f;
  c->front[2] = -1.0f;
  
  c->up[0] = 0.0f;
  c->up[1] = 1.0f;
  c->up[2] = 0.0f;

  c->yaw    = 45.0f;
  c->pitch  = -45.0f;

  mat4x4_identity(c->view);
  mat4x4_identity(c->projection);

  // setup an ortho projection
  mat4x4_ortho(c->projection, left, right, bottom, top, -100.0f, 100.0f);

  return c;
}

void ex_ortho_camera_update(ex_ortho_camera_t *cam, GLuint shader_program)
{
  // calculate view matrix based on pitch and yaw
  vec3 front;
  front[0] = cos(rad(cam->yaw)) * cos(rad(cam->pitch));
  front[1] = sin(rad(cam->pitch));
  front[2] = sin(rad(cam->yaw)) * cos(rad(cam->pitch));
  
  vec3_norm(cam->front, front);

  vec3 center;
  vec3_add(center, cam->position, cam->front);
  mat4x4_look_at(cam->view, cam->position, center, cam->up);
}

void ex_ortho_camera_draw(ex_ortho_camera_t *cam, GLuint shader_program)
{
  // shader vars
  GLuint projection_location = glGetUniformLocation(shader_program, "u_projection");
  glUniformMatrix4fv(projection_location, 1, GL_FALSE, cam->projection[0]);
  GLuint view_location = glGetUniformLocation(shader_program, "u_view");
  glUniformMatrix4fv(view_location, 1, GL_FALSE, cam->view[0]);
}