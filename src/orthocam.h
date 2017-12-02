#ifndef EX_ORTHO_CAMERA_H
#define EX_ORTHO_CAMERA_H

#define GLEW_STATIC
#include <GL/glew.h>

#include "math.h"
#include <stdbool.h>

typedef struct {
  vec3 position, front, up;
  double yaw, pitch, fov;
  mat4x4 view, projection;
  int width, height;
} ex_ortho_camera_t;

ex_ortho_camera_t* ex_ortho_camera_new(float x, float y, float z, float left, float right, float bottom, float top);

void ex_ortho_camera_update(ex_ortho_camera_t *cam, GLuint shader_program);

void ex_ortho_camera_draw(ex_ortho_camera_t *cam, GLuint shader_program);

#endif // EX_ORTHO_CAMERA_H