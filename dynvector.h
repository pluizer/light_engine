/**
Copyright (c) 2013 Richard van Roy (pluizer)

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.
**/

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
