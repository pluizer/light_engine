#ifndef __aux_h__
#define __aux_h__

#ifdef DEBUG
#define CHECK_GL() check_gl_error(__func__);
#else
#define CHECK_GL()
#endif /* DEBUG */

extern void swap_float(float* a, float* b);

extern int zeroish(float v);

#ifdef DEBUG
extern void check_gl_error(const char* func);
#endif /* DEBUG */

extern void* smalloc(size_t size);

extern void* srealloc(void* old, size_t size);

#endif /* __aux_h__ */
