#include "vec2.h"

/* uvec implementation here */

/* not finished */

static inline size_t uclamp(size_t v, size_t lo, size_t hi) {
	return v < lo ? lo : (v > hi ? hi : v);
}

struct uvec uvec_init(size_t size, size_t bytes) {
	struct uvec uv;

	uv.size = 0;
	uv.cap = size ? size + (size >> 1) : UVEC_INITIAL_CAP; 
	uv.bytes = bytes;

	uv.data = umalloc(uv.cap * uv.bytes);
	if (!uv.data)
		return (struct uvec){NULL,0,0,0};

	return uv;
}

char uvec_uninit(struct uvec *uv) {
	if (!uv) return -1;

	if (uv->data) ufree(uv->data);

	uv->size = 0;
	uv->cap = 0;

	return 0;
}

struct uvec uvec_clone(struct uvec *uv) {
	if (!uv || !uv->data) 
		return (struct uvec){NULL,0,0,0}; 
  
	struct uvec ret = uvec_init(uv->size, uv->bytes);
        if (!ret.data) 
		return (struct uvec){NULL,0,0,0};

	umemcpy(ret.data, uv->data, uv->size * uv->bytes);

	ret.size = uv->size;
	return ret;
}

/* uvec - Load My File In Bytes! */
struct uvec uvec_lmfb(int fd) { 
	struct stat st;
	if (fstat(fd, &st) == -1)
		return (struct uvec){NULL,0,0,0};

	size_t cap = st.st_size + (st.st_size >> 1);
	struct uvec uv = uvec_init(cap, 1);
  
	ssize_t n = read(fd, uv.data, st.st_size);
	if (n != st.st_size || !uv.data) {
		uvec_uninit(&uv);
		return uv;
	}

	uv.size = st.st_size;

	return uv;
}

char uvec_urealloc(struct uvec *uv, size_t cap) {
	if (!uv || !uv->data) return -1;

	void *ndat = urealloc(uv->data, cap * uv->bytes);
	if (!ndat) return -1;

	uv->data = ndat;
	uv->cap = cap;

	return 0;
}

char uvec_realize(struct uvec *uv, size_t min) {
	if (!uv || !uv->data) return -1;
	if (uv->cap < 1) uv->cap = UVEC_INITIAL_CAP;

	size_t newcap = uv->cap;

	while (newcap <= uv->size) 
		newcap = uvec_growthfunc(newcap);

#ifndef UVEC_NO_SHRINK
	while (uvec_shrinkfunc(newcap) >= uv->size && newcap > UVEC_INITIAL_CAP)
		newcap = uvec_shrinkfunc(newcap);
#endif

	if (newcap < min) newcap = min;

	return newcap != uv->cap ? uvec_urealloc(uv, uv->cap = newcap) : 0;
}

char uvec_push(struct uvec *uv, void *data, size_t elems) {
	if (!data || !uv || !uv->data) return -1;
  
	if (uvec_realize(uv, uv->size + elems) == -1)
		return -1;
  
	umemcpy(
		(uint8_t *)uv->data + (uv->size * uv->bytes), 
		data, 
		uv->bytes * elems
	);

	uv->size += elems;

	return 0;
}

char uvec_insert(struct uvec *uv, void *data, size_t elems, size_t idx) {
	if (!data || !uv || !uv->data) return -1;
  
	idx = uclamp(idx, 0, uv->size);
	if (elems == 0) return 0;

	if (uvec_realize(uv, uv->size + elems) == -1) return -1;

	umemmov(
		(uint8_t *)uv->data + (idx + elems) * uv->bytes,
		(uint8_t *)uv->data + (idx * uv->bytes),
		(uv->size - idx) * uv->bytes
	);

	umemcpy(
		(uint8_t *)uv->data + (idx * uv->bytes),
		data, 
		elems * uv->bytes
	);

	uv->size += elems;

	return 0;
}

char uvec_remove(struct uvec *uv, size_t elems, size_t idx) {
	if (!uv || !uv->data) return -1;

	idx = uclamp(idx, 0, uv->size);
	elems = uclamp(elems, 0, uv->size - idx);
	if (elems == 0) return 0;

	umemmov(
		(uint8_t *)uv->data + (idx * uv->bytes),
		(uint8_t *)uv->data + (idx + elems) * uv->bytes,
		(uv->size - (idx + elems)) * uv->bytes
	);

	uv->size -= elems;

	if (uvec_realize(uv, uv->size) == -1)
		return -1;

	return 0;
}

char uvec_take(struct uvec *uv, void *out, size_t elems, size_t idx) {
	if (!uv || !out || !uv->data) return -1;

	idx = uclamp(idx, 0, uv->size);
	elems = uclamp(elems, 0, uv->size - idx);
	if (elems == 0) return 0;

	umemcpy(out,
		(uint8_t *)uv->data + (idx * uv->bytes),
		elems * uv->bytes
	);

	umemmov(
		(uint8_t *)uv->data + (idx * uv->bytes),
		(uint8_t *)uv->data + (idx + elems) * uv->bytes,
		(uv->size - (idx + elems)) * uv->bytes
	);

	uv->size -= elems;

	if (uvec_realize(uv, uv->size) == -1) 
		return -1;

	return 0;
}

char uvec_pop(struct uvec *uv, void *out, size_t elems) {
	if (!uv || !out || !uv->data) return -1;

	elems = uclamp(elems, 0, uv->size);

	umemcpy( out, 
		(uint8_t *)uv->data + ((uv->size - elems) * uv->bytes),
		elems * uv->bytes
	);

	uv->size -= elems;

	if (uvec_realize(uv, uv->size) == -1)
		return -1;

	return 0;
}

char uvec_discard(struct uvec *uv, size_t elems) {
	if (!uv || !uv->data) return -1;

	elems = uclamp(elems, 0, uv->size);

	uv->size -= elems;

	if (uvec_realize(uv, uv->size) == -1)
		return -1;

	return 0;
}
