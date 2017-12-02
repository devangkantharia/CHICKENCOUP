#ifndef PLAYER_H
#define PLAYER_H

#include "model.h"
#include "scene.h"
#include "entity.h"

extern ex_model_t *player_model;
extern ex_entity_t *player_entity;

void player_init(ex_scene_t *s);

void player_update(double dt);

#endif // PLAYER_H