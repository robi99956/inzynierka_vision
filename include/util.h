#ifndef UTIL_H
#define UTIL_H

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#define THREAD
#define UNUSED(x) (void)x

#define NEW(type) ((type*)malloc(sizeof(type)))
#define TABLE(n, type) ((type*)calloc(sizeof(type), n))
#define RELEASE(ptr) ({if(ptr != NULL) free(ptr); NULL;})
#define REALLOC(ptr, new_size) ((typeof(ptr))realloc(ptr, new_size*sizeof(ptr[0])))

#define MAKE(type, ...) ({type* __new = NEW(type); assert(__new); *__new = (type){__VA_ARGS__}; __new;})

#ifndef ABS
#define ABS(x) ((x>0)?x:-x)
#endif

static inline uint8_t saturate(int x)
{
	if( x > 255 ) return 255;
	if( x < 0 ) return 0;

	return x;
}

#endif
