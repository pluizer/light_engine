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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "dynvector.h"
#include "aux.h"

typedef struct _DV_IndexStack
{
	unsigned* data;
	unsigned size_hint;
	unsigned size;
} DV_IndexStack;

DV_IndexStack* new_index_stack(unsigned size_hint)
{
	DV_IndexStack* is = smalloc(sizeof(DV_IndexStack));
	is->data = smalloc(sizeof(unsigned) * size_hint);
	is->size_hint = size_hint;
	is->size = 0;
	return is;
}

void free_index_stack(DV_IndexStack* is)
{
	free(is->data);
	free(is);
}

void index_stack_grow(DV_IndexStack* is)
{
	is->size_hint *= 2;
	is->data = srealloc(is->data, sizeof(unsigned) * is->size_hint);
}

void index_stack_push(DV_IndexStack* is, unsigned index)
{
	if (is->size >= is->size_hint)
	{
		index_stack_grow(is);
	}
	is->data[is->size++] = index;
}

int index_stack_empty(DV_IndexStack* is)
{
	return !is->size;
}

unsigned index_stack_pop(DV_IndexStack* is)
{
	assert(!index_stack_empty(is));
	return is->data[--is->size];
}

DV_Vector* dv_vector_new(unsigned chunk_size, unsigned size_hint)
{
	DV_Vector* dv = smalloc(sizeof(DV_Vector));
	assert(dv);
	dv->indices = smalloc(sizeof(unsigned)*size_hint);
	memset(dv->indices, 0, sizeof(unsigned)*size_hint);
	dv->available_stack = new_index_stack(size_hint);
	dv->last_stack = new_index_stack(size_hint);
	dv->size = 0;
	dv->size_hint = size_hint;
	dv->chunk_size = chunk_size;
	dv->data = smalloc(dv->chunk_size * sizeof(float) * size_hint);
	memset(dv->data, 0, dv->chunk_size * sizeof(float) * size_hint);
	return dv;
}

void dv_vector_free(DV_Vector* dv)
{
	free(dv->data);
	free(dv->indices);
	free_index_stack(dv->available_stack);
	free_index_stack(dv->last_stack);
	free(dv);
}

unsigned vector_grow(DV_Vector* dv)
{
	unsigned old_max = dv->size_hint;
	dv->size_hint *= 2;
	dv->data = srealloc(dv->data, 
			   dv->chunk_size * sizeof(float) * dv->size_hint);
	dv->indices = srealloc(dv->indices, sizeof(unsigned) * dv->size_hint);
	memset(dv->indices+old_max, 0, sizeof(unsigned)*old_max);
	return dv->size_hint - old_max;
}


unsigned dv_vector_push(DV_Vector* dv, float* chunk, unsigned* grown_by)
{
	unsigned index = index_stack_empty(dv->available_stack) 
		? dv->size : index_stack_pop(dv->available_stack);
	
	*grown_by = 0;

	/* vector full */
	if (dv->size >= dv->size_hint)
	{
		*grown_by = vector_grow(dv);
	}

	memcpy(dv->data+(dv->size*dv->chunk_size), chunk, dv->chunk_size*sizeof(float));
	dv->indices[index] = dv->size;
	dv->size++;
	index_stack_push(dv->last_stack, index);
	return index;
}

void dv_vector_remove(DV_Vector* dv, unsigned index)
{
	dv->size--;
	if (dv->indices[index] != dv->size)
	{
		memcpy(dv->data+(dv->chunk_size * dv->indices[index]),
		       dv->data+(dv->chunk_size * (dv->size)),
		       dv->chunk_size * sizeof(float));
		unsigned last = index_stack_pop(dv->last_stack);
		dv->indices[last] = dv->indices[index];
	} else {
		index_stack_pop(dv->last_stack);
	}
	index_stack_push(dv->available_stack, index);
}

void dv_vector_change(DV_Vector* dv, unsigned index, float* chunk)
{
	memcpy(dv->data+(dv->chunk_size * dv->indices[index]),
	       chunk,
	       dv->chunk_size * sizeof(float));
}

float* dv_vector_ref(DV_Vector* dv, unsigned index)
{
	return dv->data+(dv->chunk_size * dv->indices[index]);
}

unsigned dv_vector_current_capacity(DV_Vector* dv)
{
	return dv->size_hint;
}
