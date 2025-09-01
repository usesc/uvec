#include "vec.h"

/* uvec implementation here - usesc 2025: MIT License */

static inline size_t uclamp(size_t v, size_t lo, size_t hi) {
  if (v < lo) return lo;
  if (v > hi) return hi;
  return v;
}

char uvec_setvalidity(struct uvec *uv) {
  if (!uv) return UVEC_INVAL;

  if (!uv->data || uv->stat == UVEC_INVAL) {
    uv->stat = UVEC_INVAL;
    return UVEC_INVAL;
  }

  uv->stat = UVEC_VALID;
  return UVEC_VALID;
}

struct uvec uvec_init(size_t size, size_t bytes) {
  struct uvec uv;

  uv.size = 0;
  uv.cap = size ? size : UVEC_INITIAL_CAP; 
  uv.bytes = bytes;

  uv.data = malloc(uv.cap * uv.bytes);
  if (!uv.data) {
    return uvec_badvec;
  }

  uv.stat = UVEC_VALID;
  return uv;
}

int uvec_uninit(struct uvec *uv) {
  if (!uv) return UVEC_FAILED;

  free(uv->data);
  uv->data = NULL;

  uv->size = 0;
  uv->cap = 0;
  uv->stat = UVEC_INVAL;
  return UVEC_SUCCESS;
}

int uvec_realloc(struct uvec *uv, size_t cap) {
  if (!uv) return UVEC_FAILED;
  uvec_setvalidity(uv);
  if (uvec_isinv(*uv)) return UVEC_FAILED;

  void *ndat = realloc(uv->data, cap * uv->bytes);
  if (!ndat) return UVEC_FAILED;

  uv->data = ndat;
  uv->cap = cap;

  return UVEC_SUCCESS;
}

int uvec_realize(struct uvec *uv, size_t min) {
  if (!uv) return UVEC_FAILED;
  uvec_setvalidity(uv);
  if (uvec_isinv(*uv)) return UVEC_FAILED;

  char c = 0;

  if (uv->cap < 1) uv->cap = UVEC_INITIAL_CAP;

  while (uv->size >= uv->cap) {
    uv->cap *= UVEC_ACCELERATION;
    c = 1;
  }

  while (uv->cap / UVEC_DECELERATION >= uv->size && uv->cap > UVEC_INITIAL_CAP) {
    uv->cap /= UVEC_DECELERATION;
    c = 1;
  }

  if (uv->cap < min) {
    uv->cap = min;
    c = 1;
  }

  if (c) {
    return uvec_realloc(uv, uv->cap);
  }

  return UVEC_SUCCESS;
}

int uvec_push(struct uvec *uv, void *data, size_t elems) {
  if (!data || !uv) return UVEC_FAILED;
  uvec_setvalidity(uv);
  if (uvec_isinv(*uv)) return UVEC_FAILED;
  
  int rl = uvec_realize(uv, uv->size + elems);
  if (rl == UVEC_FAILED) return UVEC_FAILED;

  void *dest = (unsigned char *)uv->data + (uv->size * uv->bytes);
  memcpy(dest, data, uv->bytes * elems);

  uv->size += elems;
  return UVEC_SUCCESS;
}

int uvec_insert(struct uvec *uv, void *data, size_t elems, size_t idx) {
  if (!data || !uv) return UVEC_FAILED;
  uvec_setvalidity(uv);
  if (uvec_isinv(*uv)) return UVEC_FAILED;
  
  idx = uclamp(idx, 0, uv->size);
  if (elems == 0) return UVEC_SUCCESS;

  int rl = uvec_realize(uv, uv->size + elems);
  if (rl == UVEC_FAILED) return UVEC_FAILED;

  size_t offset = idx * uv->bytes;

  memmove(
    (unsigned char *)uv->data + offset + elems * uv->bytes,
    (unsigned char *)uv->data + offset,
    (uv->size - idx) * uv->bytes
  );

  memcpy((unsigned char *)uv->data + offset, data, elems * uv->bytes);

  uv->size += elems;

  return UVEC_SUCCESS;
}

int uvec_remove(struct uvec *uv, size_t elems, size_t idx) {
  if (!uv) return UVEC_FAILED;
  uvec_setvalidity(uv);
  if (uvec_isinv(*uv)) return UVEC_FAILED;

  idx = uclamp(idx, 0, uv->size);
  elems = uclamp(elems, 0, uv->size - idx);
  if (elems == 0) return UVEC_SUCCESS;

  int rl = uvec_realize(uv, uv->size + elems);
  if (rl == UVEC_FAILED) return UVEC_FAILED;

  memmove(
    (unsigned char *)uv->data + (idx * uv->bytes),
    (unsigned char *)uv->data + (idx * uv->bytes) + (elems * uv->bytes),
    (uv->size * uv->bytes) - ((idx * uv->bytes) + (elems * uv->bytes))
  );

  uv->size -= elems;

  return UVEC_SUCCESS;
}

int uvec_take(struct uvec *uv, void *out, size_t elems, size_t idx) {
  if (!uv || !out) return UVEC_FAILED;
  uvec_setvalidity(uv);
  if (uvec_isinv(*uv)) return UVEC_FAILED;

  idx = uclamp(idx, 0, uv->size);
  elems = uclamp(elems, 0, uv->size - idx);
  if (elems == 0) return UVEC_SUCCESS;

  int rl = uvec_realize(uv, uv->size + elems);
  if (rl == UVEC_FAILED) return UVEC_FAILED;

  memcpy(
    out,
    (unsigned char *)uv->data + (idx * uv->bytes),
    elems * uv->bytes
  );

  memmove(
    (unsigned char *)uv->data + (idx * uv->bytes),
    (unsigned char *)uv->data + (idx * uv->bytes) + (elems * uv->bytes),
    (uv->size * uv->bytes) - ((idx * uv->bytes) + (elems * uv->bytes))
  );

  uv->size -= elems;

  return UVEC_SUCCESS;
}

int uvec_pop(struct uvec *uv, void *out, size_t elems) {
  if (!uv || !out) return UVEC_FAILED;
  uvec_setvalidity(uv);
  if (uvec_isinv(*uv)) return UVEC_FAILED;

  elems = uclamp(elems, 0, uv->size);

  memcpy(
    out, 
    (unsigned char *)uv->data + (uv->bytes * (uv->size - elems)),
    elems * uv->bytes
  );

  uv->size -= elems;

  int rl = uvec_realize(uv, uv->size + elems);
  if (rl == UVEC_FAILED) return UVEC_FAILED;

  return UVEC_SUCCESS;
}

int uvec_discard(struct uvec *uv, size_t elems) {
  if (!uv) return UVEC_FAILED;
  uvec_setvalidity(uv);
  if (uvec_isinv(*uv)) return UVEC_FAILED;

  elems = uclamp(elems, 0, uv->size);

  uv->size -= elems;

  int rl = uvec_realize(uv, uv->size + elems);
  if (rl == UVEC_FAILED) return UVEC_FAILED;
  
  return UVEC_SUCCESS;
}

struct uvec uvec_clone(struct uvec *uv) {
  if (!uv) return uvec_badvec; 
  uvec_setvalidity(uv);
  if (uvec_isinv(*uv)) return uvec_badvec;
  
  struct uvec ret = uvec_init(uv->size, uv->bytes);
  if (uvec_isinv(ret)) return ret;

  memcpy(ret.data, uv->data, uv->size * uv->bytes);

  ret.size = uv->size;
  return ret;
}
