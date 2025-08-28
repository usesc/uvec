#include <stdlib.h>
#include <stdint.h>

/*
UNTESTED CODE

TODO:
test
*/

/*
TODO:
look at this guys repo
https://github.com/Mashpoe/c-vector/blob/master/vec.c
*/

struct vec {
  void *data;
  size_t elem_size;
  size_t size;
  size_t capacity;
};

void *vec_init(struct vec *v, size_t size, size_t elem_size) {
  v->size = size;
  v->capacity = size ? size : 1; 
  v->elem_size = elem_size;
  v->data = malloc(v->capacity * v->elem_size);
  if (!v->data) {
    v->size = 0;
    v->capacity = 0;
    return NULL;
  }
  return v->data;
}

void vec_uninit(struct vec *v) {
  if (!v) return;
  free(v->data);
  v->data = NULL;
  v->size = 0;
  v->capacity = 0;
}

void vec_realloc(struct vec *v, size_t capacity) {
  v->capacity = capacity;
  v->data = realloc(v->data, v->capacity * v->elem_size);
}

void vec_realize(struct vec *v) {
  if (v->size >= v->capacity) {
    v->capacity = v->capacity * 2;
    if (v->capacity < 1) v->capacity = 1;
    vec_realloc(v, v->capacity);
  }
  else if (v->size < v->capacity/2) {
    v->capacity = v->capacity / 2;
    if (v->capacity < 1) v->capacity = 1;
    vec_realloc(v, v->capacity);
  }
}

void vec_push(struct vec *v, void *data, size_t elems) {
  vec_realize(v);

  void *dest = (char *)v->data + (v->size * v->elem_size);
  memcpy(dest, data, v->elem_size * elems);

  v->size+=elems;
}

void vec_push1(struct vec *v, void *data) {
  vec_push(v, data, 1);
}
