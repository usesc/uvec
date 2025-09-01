#ifndef UVEC_H
#define UVEC_H

#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* if you are passing a struct pointer to these functions you need to dereference */
#define uvec_at(uvec,pos,type) (((type *)(uvec).data)[pos])
#define uvec_first(uvec,type) (((type *)(uvec).data)[0])
#define uvec_final(uvec,type) (((type *)(uvec).data)[(uvec).size-1])

#define uvec_isinv(uvec) ((uvec).stat == UVEC_INVAL)
#define uvec_isval(uvec) ((uvec).stat == UVEC_VALID)
#define uvec_isemt(uvec) ((uvec).size == 0)

/* initial capacity, growth and shrink rate of the vector */
#ifndef UVEC_INITIAL_CAP
#define UVEC_INITIAL_CAP 32
#endif
#ifndef UVEC_ACCELERATION
#define UVEC_ACCELERATION 2
#endif
#ifndef UVEC_DECELERATION
#define UVEC_DECELERATION 2
#endif

/* internal codes */
#define UVEC_FAILED -1337
#define UVEC_SUCCESS 0

#define UVEC_INVAL 10
#define UVEC_VALID 20

/* bad struct - returns on error */
#define uvec_badvec (struct uvec){NULL, 0, 0, 0, UVEC_INVAL}

/* uvec struct, basically your vector, filled with metadata */
struct uvec {
  void * data; /* data pointer */
  size_t bytes; /* element size in bytes */
  size_t size; /* size of effective data, in elements */
  size_t cap; /* size of allocated data, in elements */
  char   stat; /* status of vector */
};

/* simple 1 element operations */
#define uvec_push1(uv,dat) uvec_push(uv, dat, 1)
#define uvec_pop1(uv,out) uvec_pop(uv, out, 1)
#define uvec_insert1(uv,dat,idx) uvec_insert(uv, dat, 1, idx)
#define uvec_remove1(uv,idx) uvec_remove(uv, 1, idx)
#define uvec_discard1(uv) uvec_discard(uv, 1)
#define uvec_take1(uv,out,idx) uvec_take(uv, out, 1, idx)

/* mostly internal function used for error checking */
char uvec_setvalidity(struct uvec *uv);

/* initializes the vector */
struct uvec uvec_init(size_t size, size_t bytes);

/* uninitializes the vector */
int uvec_uninit(struct uvec *uv);

/* reallocates the capacity for the vector */
int uvec_realloc(struct uvec *uv, size_t cap);

/* knows when to reallocate the capacity for the vector, and does so */
int uvec_realize(struct uvec *uv, size_t min);

/* add some data for a number of elements to the end */
int uvec_push(struct uvec *uv, void *data, size_t elems);

/* add some data for a number of elements at an index  */
int uvec_insert(struct uvec *uv, void *data, size_t elems, size_t idx);

/* subtract some data for a number of elements at an index and dont return the data */
int uvec_remove(struct uvec *uv, size_t elems, size_t idx);

/* subtract some data for a number of elements at an index and return the data */
int uvec_take(struct uvec *uv, void *out, size_t elems, size_t idx);

/* subtract some data for a number of elements at the end and return the data */
int uvec_pop(struct uvec *uv, void *out, size_t elems);

/* subtract some data for a number of elements at the end and dont return the data */
int uvec_discard(struct uvec *uv, size_t elems);

/* clone a vector into another completely independent vector */
struct uvec uvec_clone(struct uvec *uv);

#endif
