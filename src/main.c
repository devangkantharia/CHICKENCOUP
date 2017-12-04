#include <stdlib.h>
#include <stdio.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <ft2build.h>

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#endif

/* TITLE: CHICKEN COUP */

// engine stuffs
#include "shader.h"
#include "window.h"
#include "scene.h"
#include "iqm.h"
#include "model.h"
#include "sound.h"
#include "text.h"

// non engine includes
#include "player.h"
#include "bby.h"

// scene stuff
GLuint shader;
ex_scene_t *scene;
ex_model_t *level = NULL, *menu_text = NULL, *egg = NULL;
ex_ortho_camera_t *camera;

ex_source_t *sound_ding;

int bby_alive = 0;

int level_reset = 0;
int level_index = 0;
const char *levels[] = {
  "data/level1.iqm",
  "data/level2.iqm",
  "data/level3.iqm",
};

// timestep
const double phys_delta_time = 1.0 / 120.0;
const double slowest_frame = 1.0 / 15.0;
double delta_time, last_frame_time, accumulator = 0.0;

// level changing/state stuffs
int changing_level = 0;

void do_frame();
void at_exit();
void next_level();

int main()
{
  // it begins
  ex_window_init(640, 480, "CHICKEN COUP");
  last_frame_time = glfwGetTime();

  // main shader program
  shader = ex_shader_compile("data/shaders/shader.vs", "data/shaders/shader.fs");

  // init the scene
  scene = ex_scene_new(shader);

  // add a sun
  scene->sun = ex_dir_light_new((vec3){0.4f, 30.0f, -20.0f}, (vec3){0.4f, 0.4f, 0.4f}, 1);

  // load the first (menu) level
  level = ex_iqm_load_model(scene, "data/menu.iqm", 1);
  list_add(scene->model_list, level);
  menu_text = ex_iqm_load_model(scene, "data/menu_text.iqm", 0);
  list_add(scene->model_list, menu_text);
  menu_text->is_lit = 0;

  // water will be present on all levels
  ex_model_t *water = ex_iqm_load_model(scene, "data/water.iqm", 0);
  list_add(scene->model_list, water);
  water->position[1] -= 5.0f;

  // setup a camera
  const float size = 15.0f;
  camera = ex_ortho_camera_new(0.0f, 0.0f, 0.0f, -size, size, -size, size);
  scene->ortho_camera = camera;

  // init all the things
  player_init(scene);
  bby_manager_init(scene, 0);

  // sounds
  ex_sound_master_volume(1.0f);
  sound_ding = ex_sound_load_source("data/sound/ding.ogg", EX_SOUND_OGG, 0);
  alSourcef(sound_ding->id, AL_GAIN, 0.5);

  // add some bby chickens to first level
  for (int i=0; i<3; i++)
    bby_new((vec3){-1.0f, 5.0f+i*1.0f, cos(i)});

  // start game loop
#ifdef __EMSCRIPTEN__
  emscripten_set_main_loop(do_frame, 0, 0);
#else
  while (!glfwWindowShouldClose(display.window)) {
    do_frame();
  
    if (ex_keys_down[GLFW_KEY_ESCAPE])
      break;
  }

  at_exit();
#endif
}

void do_frame()
{
  // dt crapola
  double current_frame_time = (double)glfwGetTime();
  delta_time = current_frame_time - last_frame_time;
  last_frame_time = current_frame_time;

    // prevent spiral of death
  if (delta_time > slowest_frame)
    delta_time = slowest_frame;

  accumulator += delta_time;
  while (accumulator >= phys_delta_time) {
    ex_window_begin();
    
    // update entities
    player_update(phys_delta_time);
    bby_manager_update(phys_delta_time);

    // update the game scene
    ex_scene_update(scene, phys_delta_time);

    // handle egg lol
    if (egg != NULL && player_entity != NULL) {
      // spin dat egg
      egg->rotation[1] += 100.0f * phys_delta_time;

      // distance to egg
      vec3 temp;
      vec3_sub(temp, egg->position, player_entity->position);
      float dist = vec3_len(temp);

      // collect egg
      if (dist < 1.0f) {
        // play some funky sound
        alSourcePlay(sound_ding->id);

        // remove egg model
        scene->model_list = list_remove(scene->model_list, egg);
        ex_model_destroy(egg);
        egg = NULL;
      }
    }

    if (changing_level)
      next_level();

    accumulator -= phys_delta_time;
  }

  // handle respawns
  if (player_entity->position[1] < -25.0f) {
    if (level_index > 0)
      level_index--;

    
    player_entity->position[1] = -20.0f;
    player_entity->velocity[1] = 0.0f;
    level_reset = 1;
    next_level();
  }

  // render err'thing
  ex_scene_draw(scene);

  ex_window_end();
}

void at_exit()
{
  ex_scene_destroy(scene);
}

// hard-coded fustercluck of shit, turn back now
void next_level()
{
  if (level_index == 3)
    level_index = 0;

  if (!changing_level) {
    changing_level = 1;

    // reset collision data
    scene->coll_tree = ex_octree_reset(scene->coll_tree);
    free(scene->coll_vertices);
    scene->coll_vertices   = NULL;
    list_destroy(scene->coll_list);
    scene->coll_list = list_new();
    scene->collision_built = 0;
    scene->coll_vertices_last = 0;
  
    if (egg != NULL) {
      scene->model_list = list_remove(scene->model_list, egg);
      ex_model_destroy(egg);
      egg = NULL;
    }

    bby_alive = 0;
  }

  if (level == NULL)
    return;

  for (int i=0; i<MAX_BBY; i++) {
    if (bby_chickens[i] == NULL)
      continue;

    bby_chickens[i]->entity->position[1] -= 50.0f * phys_delta_time;
  }

  if (changing_level == 1) {
    level->position[1] -= 100.0f * phys_delta_time;

    if (menu_text != NULL)
      menu_text->position[1] -= 100.0f * phys_delta_time;
  }

  if (changing_level == 1 && player_entity->position[1] < -25.0f) {
    scene->model_list = list_remove(scene->model_list, level);
    ex_model_destroy(level);
    
    // reset bby chickens
    for (int i=0; i<MAX_BBY; i++) {
      if (bby_chickens[i] != NULL && (!bby_chickens[i]->spawner || !level_reset)) {
        bby_alive++;
      }
    }
    bby_manager_init(scene, 1);

    level = ex_iqm_load_model(scene, levels[level_index++], 1);
    level->position[1] = -50.0f;
    ex_model_update(level, phys_delta_time);
    list_add(scene->model_list, level);
    changing_level = 2;
  
    // destroy menu models
    if (menu_text != NULL) {
      scene->model_list = list_remove(scene->model_list, menu_text);
      ex_model_destroy(menu_text);
      menu_text = NULL;
    }
  }

  if (changing_level == 2) {
    player_entity->position[0] = 0.0f;
    player_entity->position[1] = 25.0f;
    player_entity->position[2] = 0.0f;
    player_entity->velocity[0] = 0.0f;
    player_entity->velocity[1] = 0.0f;
    player_entity->velocity[2] = 0.0f;
    player_poo_timer = (double)glfwGetTime();
    player_velocity = 0.0f;
    level->position[1] += 100.0f * phys_delta_time;

    for (int i=0; i<MAX_BBY; i++) {
      if (bby_chickens[i] != NULL) {
        bby_chickens[i]->entity->velocity[1] = 0.0f;
        bby_chickens[i]->entity->position[1] = 35.0f;
      }
    }
  }

  if (changing_level == 2 && level->position[1] >= 0.0f) {
    level->position[1] = 0.0f;
    changing_level = 0;

    // add some bby chickens
    for (int i=0; i<bby_alive; i++) 
      bby_new((vec3){-1.0f, 50.0f+i*1.0f, cos(i)});

    if (egg != NULL)
      list_add(scene->model_list, egg);

    level_reset = 0;
  }
}