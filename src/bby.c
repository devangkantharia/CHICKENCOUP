#include "bby.h"
#include "iqm.h"
#include "sound.h"
#include "player.h"
#include <stdlib.h>
#include <math.h>

const float friction = 50.0f;
ex_scene_t *scene;
extern int level_index;
extern ex_model_t *level;
extern int changing_level;
extern int level_reset;
extern void next_level();

// bby_manager stuffs
bby_t *bby_chickens[MAX_BBY];
ex_source_t *sound_chirp[3] = {NULL, NULL, NULL};

bby_t* bby_new(vec3 position)
{
  // make a new bby
  bby_t* c = malloc(sizeof(bby_t));

  // load the model
  // make this instanced if i get time
  c->model = ex_iqm_load_model(scene, "data/bbychick.iqm", 0);
  list_add(scene->model_list, c->model);

  // init the entity
  c->entity = ex_entity_new(scene, (vec3){0.25f, 0.35f, 0.25f});
  memcpy(c->entity->position, position, sizeof(vec3));
  memcpy(c->model->position, position, sizeof(vec3));
  c->velocity = 0.0f;
  c->move_speed = 0.0f;

  // ez way of handling win condition
  c->end = 0;
  c->spawner = 0;

  // slightly randomy scaling for variation
  float s = (float)rand()/(float)RAND_MAX;
  c->model->scale *= 0.5f + MAX(0.0f, MIN(s, 0.5f));
  ex_model_update(c->model, 1.0f);

  // add to bby list
  for (int i=0; i<MAX_BBY; i++) {
    if (bby_chickens[i] == NULL) {
      bby_chickens[i] = c;
      return c;
    }
  }

  // no room to add us :((
  ex_entity_destroy(c->entity);
  scene->model_list = list_remove(scene->model_list, c->model);
  ex_model_destroy(c->model);
  free(c);
}

void bby_update(bby_t *c, double dt)
{
  if (c == NULL || c->entity == NULL || c->model == NULL || c->end)
    return;

  // distance to player
  vec3 temp;
  vec3_sub(temp, c->entity->position, player_entity->position);
  float dist = vec3_len(temp);

  // set chrip volume based on distance
  float vol = MAX(0.0f, MIN(1.0f-(dist/10.0f), 1.0f));

  // randomly chirp (dirty dirty)
  if ((rand() % 500) > 498) {
    int id = rand() % 3;
    alSourcef(sound_chirp[id]->id, AL_GAIN, 0.25f * vol);
    alSourcePlay(sound_chirp[id]->id);
  }
  
  
  // set following if close
  if (dist <= 10.0f)
    c->following = 1;
  else
    c->following = 0;

  // idle state
  vec3_sub(temp, c->entity->position, player_entity->position);
  dist = vec3_len((vec3){temp[0], 0.0f, temp[2]});
  if (c->entity->grounded && !c->following && dist >= 30.0f)
    return;

  ex_entity_update(c->entity, dt);
  
  // update model position
  memcpy(c->model->position, c->entity->position, sizeof(vec3));
  
  // physics
  if (c->entity->grounded) {
    // grounded, apply friction
    vec3_scale(temp, c->entity->velocity, friction * dt);
    vec3_sub(c->entity->velocity, c->entity->velocity, temp);

    c->move_speed = 150.0f;
    c->model->rotation[0] = 0.0f;
  } else {
    // add gravity
    c->entity->velocity[1]  -= (50.0f * dt);
    c->move_speed = 10.0f;
  }

  if (c->following) {
    // get direction to player
    vec3_sub(temp, player_entity->position, c->entity->position);
    vec3_norm(temp, temp);

    // scale dir by speed
    vec3_scale(temp, temp, c->move_speed * dt);
    temp[1] = 0.0f;

    // do a lil' bby chicken hop
    if (c->entity->grounded) {
      c->entity->velocity[1] = 5.0f;
      if (c->wobble)
        c->model->rotation[0] = 20.0f;
      else
        c->model->rotation[0] = -20.0f;
      c->wobble = !c->wobble;
    }

    // move in direction
    vec3_add(c->entity->velocity, c->entity->velocity, temp);
  } else {
    // do a lil' bby chicken hop
    if (c->entity->grounded) {
      c->entity->velocity[1] = 5.0f;
      if (c->wobble)
        c->model->rotation[0] = 10.0f;
      else
        c->model->rotation[0] = -10.0f;
      c->wobble = !c->wobble;
    }
  }

  // rotate towards velocity
  c->model->rotation[1] = degrees(atan2(c->entity->velocity[2], c->entity->velocity[0]));

  // bounce off other entities
  for (int i=0; i<MAX_BBY+1; i++) {
    if (i < MAX_BBY && bby_chickens[i] == NULL)
      continue;

    ex_entity_t *other;

    if (i < MAX_BBY)
      other = bby_chickens[i]->entity;
    else
      other = player_entity;
    
    if (other == NULL || other == c->entity)
      continue;

    // direction to eachother
    vec3_sub(temp, c->entity->position, other->position);

    // distance to eachother
    float dist = vec3_len(temp);

    // dist smaller than combined radius? colliding
    if (dist <= c->entity->radius[0] + other->radius[0]) {
      vec3_norm(temp, temp);
      vec3_scale(temp, temp, (dist*200.0f) * dt);
      vec3_add(c->entity->velocity, c->entity->velocity, temp);
      if (other != player_entity)
        vec3_sub(other->velocity, other->velocity, temp);
    }
  }

  // check end region
  ex_rect_t r;
  vec3_sub(r.min, c->entity->position, c->entity->radius);
  vec3_add(r.max, c->entity->position, c->entity->radius);
  if (level != NULL && ex_aabb_aabb(level->end_bounds, r)) {
    c->end = 1;
    c->model->position[1] = -100.0f;
    c->entity->position[1] = -100.0f;

    // check if win
    // int win = 1;
    // for (int i=0; i<MAX_BBY; i++) {
      // if (bby_chickens[i] != NULL && !bby_chickens[i]->end)
        // win = 0;
    // }

    // good job pleb
    // if (win)
      // next_level();
  }

  // check if we are to die
  if (c->entity->position[1] < -150.0f && !changing_level) {
    if (level_index > 0)
      level_index--;
    
    player_entity->position[1] = -20.0f;
    player_entity->velocity[1] = 0.0f;
    level_reset = 1;
    next_level();
  }
}

void bby_manager_init(ex_scene_t *s, int reset)
{
  scene = s;

  // load sounds
  if (sound_chirp[0] == NULL)
    sound_chirp[0] = ex_sound_load_source("data/sound/chirp1.ogg", EX_SOUND_OGG, 0);
  if (sound_chirp[1] == NULL)
    sound_chirp[1] = ex_sound_load_source("data/sound/chirp2.ogg", EX_SOUND_OGG, 0);
  if (sound_chirp[2] == NULL)
    sound_chirp[2] = ex_sound_load_source("data/sound/chirp2.ogg", EX_SOUND_OGG, 0);
  
  for (int i=0; i<MAX_BBY; i++) {
    if (reset) {
      if (bby_chickens[i] != NULL) {
        bby_t *c = bby_chickens[i];
        ex_entity_destroy(c->entity);
        scene->model_list = list_remove(scene->model_list, c->model);
        ex_model_destroy(c->model);
        free(c);
        c = NULL;
      }
    }

    bby_chickens[i] = NULL;
  }
}

void bby_manager_update(double dt)
{
  // update all chickens
  for (int i=0; i<MAX_BBY; i++) {
    if (bby_chickens[i] != NULL) {
      bby_update(bby_chickens[i], dt);
    }
  }
}