#ifndef INPUTBUFFER_H_
#define INPUTBUFFER_H_
#include <sys/types.h>

typedef struct inputbuffer { 
	char* buffer;
	size_t buffer_length;
	ssize_t input_length;
} inputbuffer;
typedef struct inputbuffer * InputBuffer;

InputBuffer createInputBuffer();
void deleteInputBuffer();
void readInput(InputBuffer);

#endif
