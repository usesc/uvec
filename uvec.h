#ifndef UVEC_H
#define UVEC_H

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>

#include <sys/mman.h>
#include <sys/stat.h>

/* Define UVEC_NO_SHRINK if you dont want your vectors to shrink */

/* if you are passing a struct pointer to these functions you need to dereference */
#define uvec_at(uvec,pos,type) (((type *)(uvec).data)[pos])
#define uva(uvec,pos,type) (((type *)(uvec).data)[pos])
#define uvec_first(uvec,type) (((type *)(uvec).data)[0])
#define uvec_final(uvec,type) (((type *)(uvec).data)[(uvec).size-1])
#define uvec_isempty(uvec) ((uvec).size == 0)

/* initial capacity, growth and shrink rate of the vector */

/* 
 * You can use -
 * logarithmic growth and shrinking (y=log(x))
 * square roots (y=sqrt(x)) 
 * exponential - default (y=x^2)
 * all sorts of stuff 
 */

#ifndef UVEC_INITIAL_CAP
#define UVEC_INITIAL_CAP 32
#endif

static inline size_t uvec_growthdefault(size_t old_cap) { 
	return old_cap * 2; 
}

static inline size_t uvec_shrinkdefault(size_t old_cap) {
	size_t new_cap = old_cap / 3;
	return new_cap < UVEC_INITIAL_CAP ? UVEC_INITIAL_CAP : new_cap;
}
        
#ifndef uvec_growthfunc
#define uvec_growthfunc uvec_growthdefault
#endif

#ifndef uvec_shrinkfunc
#define uvec_shrinkfunc uvec_shrinkdefault
#endif

#if defined(__clang__) || defined(__GNUC__) 
#define umemcpy(dest, src, n) __builtin_memcpy(dest, src, n)
#define umemmov(dest, src, n) __builtin_memmove(dest, src, n)
#else
#define umemcpy(dest, src, n) memcpy(dest, src, n)
#define umemmov(dest, src, n) memmove(dest, src, n)
#endif

#ifndef umalloc
#define umalloc malloc
#endif
#ifndef ufree
#define ufree free
#endif
#ifndef urealloc
#define urealloc realloc
#endif

/* uvec struct, basically your vector, filled with metadata */
struct uvec {
	void * data; /* data pointer */
	size_t bytes; /* element size in bytes */
	size_t size; /* size of effective data, in elements, should really be named "filled" */
	size_t cap; /* size of allocated data, in elements */
};

/* simple 1 element operations */
#define uvec_push1(uv,dat) uvec_push(uv, dat, 1)
#define uvec_pop1(uv,out) uvec_pop(uv, out, 1)
#define uvec_insert1(uv,dat,idx) uvec_insert(uv, dat, 1, idx)
#define uvec_remove1(uv,idx) uvec_remove(uv, 1, idx)
#define uvec_discard1(uv) uvec_discard(uv, 1)
#define uvec_take1(uv,out,idx) uvec_take(uv, out, 1, idx)

/* initializes the vector */
struct uvec uvec_init(size_t size, size_t bytes);

/* uninitializes the vector */
char uvec_uninit(struct uvec *uv);

/* clone a vector into another completely independent vector */
struct uvec uvec_clone(struct uvec *uv);

/* uvec - Load My File in Bytes! */
struct uvec uvec_lmfb(int fd);

/* reallocates the capacity for the vector */
char uvec_realloc(struct uvec *uv, size_t cap);

/* knows when to reallocate the capacity for the vector, and does so */
char uvec_realize(struct uvec *uv, size_t min);

/* add some data for a number of elements to the end */
char uvec_push(struct uvec *uv, void *data, size_t elems);

/* add some data for a number of elements at an index  */
char uvec_insert(struct uvec *uv, void *data, size_t elems, size_t idx);

/* subtract some data for a number of elements at an index and dont return the data */
char uvec_remove(struct uvec *uv, size_t elems, size_t idx);

/* subtract some data for a number of elements at an index and return the data */
char uvec_take(struct uvec *uv, void *out, size_t elems, size_t idx);

/* subtract some data for a number of elements at the end and return the data */
char uvec_pop(struct uvec *uv, void *out, size_t elems);

/* subtract some data for a number of elements at the end and dont return the data */
char uvec_discard(struct uvec *uv, size_t elems);

#endif
