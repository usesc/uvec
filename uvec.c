#include "uvec.h"

/* 
 * uvec implementation here
 * usesc on github 
 */

/* not finished */

static inline size_t uclamp(size_t v, size_t lo, size_t hi) {
	return v < lo ? lo : (v > hi ? hi : v);
}

size_t uvec_growthdefault(size_t old_cap) { 
	return old_cap * 2; 
}

size_t uvec_shrinkdefault(size_t old_cap) {
	size_t new_cap = old_cap / 3;
	return new_cap < UVEC_INITIAL_CAP ? UVEC_INITIAL_CAP : new_cap;
}

struct uvec uvec_init(size_t size, size_t bytes) {
	struct uvec uv;

	uv.usize = 0;
	uv.ucapa = size ? size + (size >> 1) : UVEC_INITIAL_CAP; 
	uv.uelsz = bytes;

	uv.udata = umalloc(uv.ucapa * uv.uelsz);

	if (!uv.udata)
		return (struct uvec){NULL,0,0,0};

	return uv;
}

char uvec_uninit(struct uvec *uv) {
	if (!uv) 
                return -1;

	if (uv->udata) 
                ufree(uv->udata);

	uv->usize = 0;
	uv->ucapa = 0;

	return 0;
}

struct uvec uvec_clone(struct uvec *uv) {
	if (!uv || !uv->udata) 
		return (struct uvec){NULL,0,0,0}; 
  
	struct uvec ret = uvec_init(uv->usize, uv->uelsz);

        if (!ret.udata) 
		return (struct uvec){NULL,0,0,0};

	umemcpy(
                ret.udata, 
                uv->udata,
                uv->usize * uv->uelsz
        );

	ret.usize = uv->usize;

	return ret;
}

/* uvec - Load My File In Bytes! */
struct uvec uvec_lmfb(int fd) { 
	struct stat st;

	if (fstat(fd, &st) == -1)
		return (struct uvec){NULL,0,0,0};

	size_t cap = st.st_size + (st.st_size >> 1);
	struct uvec uv = uvec_init(cap, 1);
  
	ssize_t n = read(fd, uv.udata, st.st_size);

	if (n != st.st_size || !uv.udata) {
		uvec_uninit(&uv);
		return uv;
	}

	uv.usize = st.st_size;

	return uv;
}

char uvec_urealloc(struct uvec *uv, size_t cap) {
	if (!uv || !uv->udata) 
                return -1;

	void *ndat = urealloc(uv->udata, cap * uv->uelsz);

	if (!ndat) 
                return -1;

	uv->udata = ndat;
	uv->ucapa = cap;

	return 0;
}

char uvec_realize(struct uvec *uv, size_t min) {
	if (!uv || !uv->udata) 
                return -1;

	if (uv->ucapa < 1) 
                uv->ucapa = UVEC_INITIAL_CAP;

	size_t newcap = uv->ucapa;

	while (newcap < uv->usize) {
		newcap = uvec_growthfunc(newcap);
        }

#ifndef UVEC_NO_SHRINK

	size_t shrinked;
        while ((shrinked = uvec_shrinkfunc(newcap)) > uv->usize && shrinked > UVEC_INITIAL_CAP) {
                newcap = shrinked;
        }

#endif

	if (newcap < min) 
                newcap = min;

        if (newcap != uv->ucapa) {
                uv->ucapa = newcap;
                return uvec_urealloc(uv, newcap); 
        }

	return 0;
}

char uvec_push(struct uvec *uv, void *data, size_t elems) {
	if (!data || !uv || !uv->udata) 
                return -1;
  
	if (uvec_realize(uv, uv->usize + elems) == -1)
		return -1;
  
	umemcpy(
		(uint8_t *)uv->udata + (uv->usize * uv->uelsz), 
		data, 
		uv->uelsz * elems
	);

	uv->usize += elems;

	return 0;
}

char uvec_insert(struct uvec *uv, void *data, size_t elems, size_t idx) {
	if (!data || !uv || !uv->udata) 
                return -1;
  
	idx = uclamp(idx, 0, uv->usize);

	if (elems == 0) 
                return 0;

	if (uvec_realize(uv, uv->usize + elems) == -1) 
                return -1;

	umemmov(
		(uint8_t *)uv->udata + (idx + elems) * uv->uelsz,
		(uint8_t *)uv->udata + (idx * uv->uelsz),
		(uv->usize - idx) * uv->uelsz
	);

	umemcpy(
		(uint8_t *)uv->udata + (idx * uv->uelsz),
		data, 
		elems * uv->uelsz
	);

	uv->usize += elems;

	return 0;
}

char uvec_remove(struct uvec *uv, size_t elems, size_t idx) {
	if (!uv || !uv->udata) 
                return -1;

	idx = uclamp(idx, 0, uv->usize);
	elems = uclamp(elems, 0, uv->usize - idx);

	if (elems == 0) 
                return 0;

	umemmov(
		(uint8_t *)uv->udata + (idx * uv->uelsz),
		(uint8_t *)uv->udata + (idx + elems) * uv->uelsz,
		(uv->usize - (idx + elems)) * uv->uelsz
	);

	uv->usize -= elems;

	if (uvec_realize(uv, uv->usize) == -1)
		return -1;

	return 0;
}

char uvec_take(struct uvec *uv, void *out, size_t elems, size_t idx) {
	if (!uv || !out || !uv->udata) 
                return -1;

	idx = uclamp(idx, 0, uv->usize);
	elems = uclamp(elems, 0, uv->usize - idx);

	if (elems == 0) 
                return 0;

	umemcpy(
                out,
		(uint8_t *)uv->udata + (idx * uv->uelsz),
		elems * uv->uelsz
	);

	umemmov(
		(uint8_t *)uv->udata + (idx * uv->uelsz),
		(uint8_t *)uv->udata + (idx + elems) * uv->uelsz,
		(uv->usize - (idx + elems)) * uv->uelsz
	);

	uv->usize -= elems;

	if (uvec_realize(uv, uv->usize) == -1) 
		return -1;

	return 0;
}

char uvec_pop(struct uvec *uv, void *out, size_t elems) {
	if (!uv || !out || !uv->udata) 
                return -1;

	elems = uclamp(elems, 0, uv->usize);

	umemcpy(
                out, 
		(uint8_t *)uv->udata + (uv->usize - elems) * uv->uelsz,
		elems * uv->uelsz
	);

	uv->usize -= elems;

	if (uvec_realize(uv, uv->usize) == -1)
		return -1;

	return 0;
}

char uvec_discard(struct uvec *uv, size_t elems) {
	if (!uv || !uv->udata) 
                return -1;

	elems = uclamp(elems, 0, uv->usize);

	uv->usize -= elems;

	if (uvec_realize(uv, uv->usize) == -1)
		return -1;

	return 0;
}
