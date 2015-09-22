/**
Copyright (c) 2014 Richard van Roy (pluizer)

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

#include <stdlib.h>
#include <stdio.h>

void swap_float(float* a, float* b)
{
	float t = *a;
	*a = *b;
	*b = t;
}

int zeroish(float v)
{
	return (v < .0001 && v > -.0001);
}

#ifdef DEBUG
void check_gl_error(const char* func)
{
	GLuint err = glGetError();
	if (err != GL_NO_ERROR)
	{
		fprintf(stderr, "%s: OpenGL error: %s\n", func, gluErrorString(err));
		exit(-1);
	}
}
#endif

void* smalloc(size_t size)
{
	void* ptr = malloc(size);
	if (!ptr)
	{
		fprintf(stderr, "Out of memory!");
		exit(-1);
	}
	return ptr;
}

void* srealloc(void* old, size_t size)
{
	void* ptr = realloc(old, size);
	if (!ptr)
	{
		fprintf(stderr, "Out of memory!");
		exit(-1);
	}
	return ptr;
}
