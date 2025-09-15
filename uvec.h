#ifndef UVEC_H
#define UVEC_H

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>

#include <sys/mman.h>
#include <sys/stat.h>

/* Define UVEC_NO_SHRINK if you dont want your vectors to shrink */

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

/* simple 1 element operations */
#define uvec_push1(uv, dat) uvec_push(uv, dat, 1)
#define uvec_pop1(uv, out) uvec_pop(uv, out, 1)
#define uvec_insert1(uv, dat, idx) uvec_insert(uv, dat, 1, idx)
#define uvec_remove1(uv, idx) uvec_remove(uv, 1, idx)
#define uvec_discard1(uv) uvec_discard(uv, 1)
#define uvec_take1(uv, out, idx) uvec_take(uv, out, 1, idx)

#define uvec_at(uvec, pos, type) (((type *)(uvec).udata)[pos])
#define uva(uvec, pos, type) (((type *)(uvec).udata)[pos])
#define uvec_first(uvec, type) (((type *)(uvec).udata)[0])
#define uvec_final(uvec, type) (((type *)(uvec).udata)[(uvec).usize-1])
#define uvec_isempty(uvec) ((uvec).usize == 0)

struct uvec {
	void * udata;
	size_t uelsz;
	size_t usize;
	size_t ucapa;
};

size_t uvec_growthdefault(size_t old_cap);
size_t uvec_shrinkdefault(size_t old_cap);
        
struct uvec uvec_init(size_t size, size_t bytes);
char uvec_uninit(struct uvec *uv);

struct uvec uvec_clone(struct uvec *uv);
struct uvec uvec_lmfb(int fd);

char uvec_realloc(struct uvec *uv, size_t cap);
char uvec_realize(struct uvec *uv, size_t min);

char uvec_push(struct uvec *uv, void *data, size_t elems);
char uvec_insert(struct uvec *uv, void *data, size_t elems, size_t idx);
char uvec_remove(struct uvec *uv, size_t elems, size_t idx);
char uvec_take(struct uvec *uv, void *out, size_t elems, size_t idx);
char uvec_pop(struct uvec *uv, void *out, size_t elems);
char uvec_discard(struct uvec *uv, size_t elems);

#endif
