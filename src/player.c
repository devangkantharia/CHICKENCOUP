#include "player.h"
#include "iqm.h"
#include "list.h"
#include "window.h"
#include "sound.h"
#include "bby.h"

const float spawn_locations[] = {
  1.0f, 5.0f, 1.0f,
};

ex_scene_t *scene;
extern ex_model_t *level;

extern void next_level();

// player members
ex_model_t *player_model;
ex_entity_t *player_entity;
float player_move_speed = 0.0f;
float player_velocity = 0.0f;
const float player_friction = 2.5f;
int player_wobble = 0;
ex_source_t *sound_bop_a, *sound_bop_b;
double player_poo_timer = -1.0;
double player_ground_timer = 0.0;

void player_init(ex_scene_t *s)
{
  // keep a pointer to the scene
  scene = s;

  // load any assets we need
  player_model = ex_iqm_load_model(s, "data/bigchick.iqm", 0);
  list_add(scene->model_list, player_model);

  // init entity stuff
  player_entity = ex_entity_new(scene, (vec3){0.5f, 0.6f, 0.5f});
  memcpy(player_entity->position, &spawn_locations, sizeof(vec3));

  // load sounds
  sound_bop_a = ex_sound_load_source("data/sound/bop1.ogg", EX_SOUND_OGG, 0);
  sound_bop_b = ex_sound_load_source("data/sound/bop2.ogg", EX_SOUND_OGG, 0);
  alSourcef(sound_bop_a->id, AL_PITCH, 7);
  alSourcef(sound_bop_a->id, AL_GAIN, 0.25);
  alSourcef(sound_bop_b->id, AL_PITCH, 7);
  alSourcef(sound_bop_b->id, AL_GAIN, 0.25);
}

void player_update(double dt)
{
  ex_entity_update(player_entity, dt);

  if (player_entity->grounded)
    player_ground_timer = glfwGetTime();
  
  // update the model stuff
  memcpy(player_model->position, player_entity->position, sizeof(vec3));

  // physics
  vec3 temp;
  vec3_scale(temp, player_entity->velocity, 5.0f * dt);

  if (player_entity->grounded) {
    // grounded, apply friction
    player_velocity -= player_velocity * player_friction * dt;
    player_move_speed = 2250.0f;
    player_model->rotation[0] = 0.0f;
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
  if (ex_keys_down[GLFW_KEY_W]) { 
    if (player_velocity < 500.0f)
      player_velocity = 500.0f;

    player_velocity += player_move_speed * dt;
    if (player_entity->grounded) {
      player_entity->velocity[1] = 10.0f;
      if (player_wobble)
        alSourcePlay(sound_bop_a->id);
      else
        alSourcePlay(sound_bop_b->id);
    }
  }

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
  if (ex_keys_down[GLFW_KEY_D]) {
    player_model->rotation[1] += (30.0f * current_speed) * dt;
    if (player_velocity > 0.1f && player_entity->grounded)
      player_model->rotation[0] = 35.0f * (current_speed / 30.0f);
  }
  if (ex_keys_down[GLFW_KEY_A]) {
    player_model->rotation[1] -= (30.0f * current_speed) * dt;
    if (player_velocity > 0.1f && player_entity->grounded)
      player_model->rotation[0] = -35.0f * (current_speed / 30.0f);
  }

  // jump
  if (ex_keys_down[GLFW_KEY_SPACE] && (double)glfwGetTime()-player_ground_timer < 0.2) {
    player_entity->velocity[1] = 20.0f;
    if (player_wobble)
      player_model->rotation[0] = 15.0f;
    else
      player_model->rotation[0] = -15.0f;

    player_ground_timer = -1.0;
  }
  player_wobble = !player_wobble;

  // move cam position
  scene->ortho_camera->position[0] = player_model->position[0];
  scene->ortho_camera->position[2] = player_model->position[2];

  // meh didnt like this idea
  // check if we need 2 poo
  // if ((double)glfwGetTime() - player_poo_timer >= PLAYER_POO_TIME && player_poo_timer > 0.0 && player_entity->grounded) {
    // we gotta poop!!
    // vec3 pos = {player_entity->position[0], player_entity->position[1]+0.25f ,player_entity->position[2]};
    // bby_new(pos);

    // player_poo_timer = (double)glfwGetTime();
  // }

  // throw a bby chick
  if (ex_keys_down[GLFW_KEY_F]) {
    for (int i=0; i<MAX_BBY; i++) {
      bby_t *c = bby_chickens[i];
      if (c == NULL)
        continue;
      
      // distance to bby
      vec3_sub(temp, c->entity->position, player_entity->position);
      float dist = vec3_len(temp);

      if (dist <= 1.5f) {
        // move it to us
        memcpy(c->entity->position, player_entity->position, sizeof(vec3));
      
        // throw it
        temp[0] = cos(rad(player_model->rotation[1]));
        temp[2] = sin(rad(player_model->rotation[1]));
        temp[1] = 1.2f;
        vec3_scale(temp, temp, 15.0f);
        vec3_add(c->entity->velocity, c->entity->velocity, temp);

        ex_keys_down[GLFW_KEY_F] = 0;
        break;
      }
    }
  }

  // check end region
  ex_rect_t r;
  vec3_sub(r.min, player_entity->position, player_entity->radius);
  vec3_add(r.max, player_entity->position, player_entity->radius);
  if (level != NULL && ex_aabb_aabb(level->end_bounds, r)) {

    // check if win
    int win = 1;
    for (int i=0; i<MAX_BBY; i++) {
      if (bby_chickens[i] != NULL && !bby_chickens[i]->end)
        win = 0;
    }

    // good job pleb
    if (win)
      next_level();
  }
}