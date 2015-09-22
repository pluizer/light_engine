#ifndef __dynvector_h_
#define __dynvector_h_

struct _DV_IndexStack;

typedef struct _DV_Vector
{
	float* data;
	unsigned* indices;
	struct _DV_IndexStack* available_stack;
	struct _DV_IndexStack* last_stack;
	unsigned size;
	unsigned size_hint;
	unsigned chunk_size;
} DV_Vector;

extern DV_Vector* dv_vector_new(unsigned chunk_size, unsigned size_hint);

extern void dv_vector_free(DV_Vector* dv);

extern unsigned dv_vector_push(DV_Vector* dv, float* chunk, unsigned* grown_by);

extern void dv_vector_remove(DV_Vector* dv, unsigned index);

extern void dv_vector_change(DV_Vector* dv, unsigned index, float* chunk);

extern float* dv_vector_ref(DV_Vector* dv, unsigned index);

extern unsigned dv_vector_current_capacity(DV_Vector* dv);

#endif /* __dynvector_h_ */
