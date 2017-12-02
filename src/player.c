#include "player.h"
#include "iqm.h"
#include "list.h"
#include "window.h"

const float spawn_locations[] = {
  1.0f, 5.0f, 1.0f,
};

ex_scene_t *scene;

// player members
ex_model_t *player_model;
ex_entity_t *player_entity;
float player_move_speed = 0.0f;
float player_velocity = 0.0f;
const float player_friction = 2.5f;

void player_init(ex_scene_t *s)
{
  // keep a pointer to the scene
  scene = s;

  // load any assets we need
  player_model = ex_iqm_load_model(s, "data/bigchick.iqm", 0);
  list_add(scene->model_list, player_model);

  // init entity stuff
  player_entity = ex_entity_new(scene, (vec3){0.4f, 0.4f, 0.4f});
  memcpy(player_entity->position, &spawn_locations, sizeof(vec3));
}

void player_update(double dt)
{
  ex_entity_update(player_entity, dt);
  
  // update the model stuff
  memcpy(player_model->position, player_entity->position, sizeof(vec3));

  // physics
  vec3 temp;
  vec3_scale(temp, player_entity->velocity, 5.0f * dt);

  if (player_entity->grounded) {
    // grounded, apply friction
    player_velocity -= player_velocity * player_friction * dt;
    player_move_speed = 2250.0f;
  } else {
    // not grounded, reduce move speed
    player_move_speed = 100.0f;
  }

  // apply gravity
  if (!player_entity->grounded)
    player_entity->velocity[1]  -= (100.0f * dt);
  
  // stop things bugging out
  if (player_entity->velocity[1] <= 0.0f && player_entity->grounded)
    player_entity->velocity[1] = 0.0f;

  // movement direction
  vec3 speed;
  speed[1] = 0.0f;
  speed[0] = cos(rad(player_model->rotation[1]));
  speed[2] = sin(rad(player_model->rotation[1]));
  
  // move forwards
  if (ex_keys_down[GLFW_KEY_W])
    player_velocity += player_move_speed * dt;

  // move backwards
  if (ex_keys_down[GLFW_KEY_S])
    player_velocity -= (player_move_speed * 0.25f) * dt;

  // add local velocity to entity velocity
  vec3_scale(speed, speed, player_velocity * dt);
  player_entity->velocity[0] = speed[0];
  player_entity->velocity[2] = speed[2];

  // current x/z speed len
  memcpy(temp, player_entity->velocity, sizeof(vec3));
  temp[1] = 0.0f;
  float current_speed = vec3_len(temp);
  current_speed = MAX(5.0f, MIN(15.0f, current_speed));

  // turn left and right
  if (ex_keys_down[GLFW_KEY_D])
    player_model->rotation[1] += (30.0f * current_speed) * dt;
  if (ex_keys_down[GLFW_KEY_A])
    player_model->rotation[1] -= (30.0f * current_speed) * dt;

  if (ex_keys_down[GLFW_KEY_SPACE] && player_entity->grounded)
    player_entity->velocity[1] = 20.0f;

  // camera stuff
  if (ex_keys_down[GLFW_KEY_Q])
    scene->ortho_camera->yaw += 100.0f * dt;
  if (ex_keys_down[GLFW_KEY_E])
    scene->ortho_camera->yaw -= 100.0f * dt;

  // move cam position
  scene->ortho_camera->position[0] = player_model->position[0];
  scene->ortho_camera->position[2] = player_model->position[2];
}