#ifndef BBY_H
#define BBY_H

#define MAX_BBY 24

// BBY CHICKENZ

#include "scene.h"
#include "model.h"
#include "entity.h"

typedef struct {
  ex_model_t *model;
  ex_entity_t *entity;
  float move_speed, velocity;
  int wobble, following;
} bby_t;

extern bby_t *bby_chickens[MAX_BBY];

void bby_new(vec3 position);

void bby_update(bby_t *c, double dt);

// chicken manager

void bby_manager_init(ex_scene_t *s, int reset);

void bby_manager_update(double dt);

#endif // BBY_H