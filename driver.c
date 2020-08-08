#include <stdio.h>
#include "InputBuffer.h"
#include <stdlib.h>
#include <string.h>
void printPrompt();

int main(int argc, char* argv[]) { 
	InputBuffer ib = createInputBuffer();
	while (1) { 
		printPrompt();
		readInput(ib);

		if (strcmp(ib->buffer, ".exit") == 0) { 
			deleteInputBuffer(ib);
			exit(EXIT_SUCCESS);
		} else { 
			printf("Unrecognized command '%s'.\n", ib->buffer);
		}
	}
}

void printPrompt() { 
	printf("db > ");
}

