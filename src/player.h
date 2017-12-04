#ifndef PLAYER_H
#define PLAYER_H

#include "model.h"
#include "scene.h"
#include "entity.h"

#define PLAYER_POO_TIME 20.0

extern ex_model_t *player_model;
extern ex_entity_t *player_entity;
extern float player_velocity;
extern double player_poo_timer;
void player_init(ex_scene_t *s);

void player_update(double dt);

#endif // PLAYER_H