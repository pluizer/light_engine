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
