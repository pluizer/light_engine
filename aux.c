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
