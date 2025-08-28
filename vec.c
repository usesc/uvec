#include <stdlib.h>
#include <string.h>
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

/*
wanted functionality:

int myint;

struct vec v;
vec_init(&v, 10, sizeof(int));
vec_push1(&v, 1000); 
vec_pop(&v, &myint);
// myint = 1000
vec_remle(&v) // vec remove last element

int pos = 10; int val = 15;
vec_insert(&v, val, pos)
vec_remove(&v, pos);

vat(&v, 0); 

*/

#define vat(v, p) (v->data[p])
struct vec {
  void * data;
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

void vec_realize(struct vec *v, size_t min) {
  char c = 0;

  if (v->capacity < 1) v->capacity = 1;

  while (v->size >= v->capacity) {
    v->capacity *= 2;
    c = 1;
  }

  while (v->size < v->capacity / 2) {
    v->capacity /= 2;
    c = 1;
  }

  if (v->capacity < min) {
    v->capacity = min;
    c = 1;
  }

  if (c) vec_realloc(v, v->capacity);
}

void vec_push(struct vec *v, void *data, size_t elems) {
  if (!data) return;
  
  vec_realize(v, v->size + elems);

  void *dest = (char *)v->data + (v->size * v->elem_size);
  memcpy(dest, data, v->elem_size * elems);

  v->size+=elems;
}

void vec_push1(struct vec *v, void *data) {
  vec_push(v, data, 1);
}

/*
destination needs to be the start of the
memory block you wanna move, plus the size
of all the elements you want to insert

insert:
vec *v, void *data, size_t elems

memmove()

*/

void vec_pop(struct vec *v, void *out, size_t elems) {
  if (elems > v->size) elems = v->size;

  if (out && elems > 0) {
    void *src = (char *)v->data + (v->elem_size * (v->size - elems));
    memcpy(out, src, elems * v->elem_size);
  }

  v->size -= elems;
  vec_realize(v, v->size);
}
