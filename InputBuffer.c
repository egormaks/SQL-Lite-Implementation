#include <stdio.h>
#include "InputBuffer.h"
#include <stdlib.h>

InputBuffer createInputBuffer() { 
	InputBuffer inputBuffer = malloc(sizeof(inputbuffer));
	inputBuffer->buffer = NULL;
	inputBuffer->buffer_length = 0;
	inputBuffer->input_length = 0;
	return inputBuffer;
}

void deleteInputBuffer(InputBuffer ib) { 
	free(ib->buffer);
	free(ib);
}

void readInput(InputBuffer ib) { 
	ssize_t bytesRead = getline(&(ib->buffer),&(ib->buffer_length), stdin);
	if (bytesRead <= 0) {
		printf("Error reading input\n");
		exit(1);
	} 

	ib->input_length = bytesRead - 1;
	ib->buffer[bytesRead - 1] = 0;
}	




