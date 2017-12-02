#include <stdlib.h>
#include <stdio.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <ft2build.h>

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#endif

#include "shader.h"
#include "window.h"
#include "scene.h"

// scene stuff
GLuint shader;
ex_scene_t *scene;

// timestep
const double phys_delta_time = 1.0 / 120.0;
const double slowest_frame = 1.0 / 15.0;
double delta_time, last_frame_time, accumulator = 0.0;


void do_frame();
void at_exit();

int main()
{
  // it begins
  ex_window_init(640, 480, "LD40");

  // main shader program
  shader = ex_shader_compile("data/shaders/shader.vs", "data/shaders/shader.fs");

  // init the scene
  scene = ex_scene_new(shader);

  last_frame_time = glfwGetTime();

  // start game loop
#ifdef __EMSCRIPTEN__
  emscripten_set_main_loop(do_frame, 0, 0);
#else
  while (!glfwWindowShouldClose(display.window)) {
    do_frame();
  }

  at_exit();
  ex_scene_destroy(scene);
#endif
}

void do_frame()
{
  // dt crapola
  double current_frame_time = (double)glfwGetTime();
  delta_time = current_frame_time - last_frame_time;
  last_frame_time = current_frame_time;

  accumulator += delta_time;
  while (accumulator >= phys_delta_time) {
    ex_window_begin();
    
    // update the game scene
    ex_scene_update(scene, phys_delta_time);

    accumulator -= phys_delta_time;
  }

  // render err'thing
  ex_scene_draw(scene);

  ex_window_end();
}

void at_exit()
{
  
}