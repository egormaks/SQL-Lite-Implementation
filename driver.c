#include <stdio.h>
#include "InputBuffer.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef enum {
	META_COMMAND_SUCCESS,
	META_COMMAND_UNRECOGNIZED_COMMAND
} MetaCommandResult;

typedef enum {
	PREPARE_SUCCESS,
	PREPARE_UNRECOGNIZED_STATEMENT
} PrepareResult;

typedef enum {
	STATEMENT_INSERT,
	STATEMENT_SELECT
} StatementType;

typedef struct { StatementType type; } Statement;

void printPrompt();
MetaCommandResult doMetaCommand(InputBuffer);
PrepareResult prepareStatement();
void executeStatement(Statement * statement);

int main(int argc, char* argv[]) { 
	InputBuffer ib = createInputBuffer();
	time_t t;
	time(&t);
	printf("Egor's SQLite version 1.0 %s\n", ctime(&t));
	printf("Enter \".help\" for usage hints.\n");
	printf("Connected to a transient in-memory database.\n");
	printf("Use \"open FILENAME\" to reopen on a persistent database\n");
	while (1) { 
		printPrompt();
		readInput(ib);

		if (ib->buffer[0] == '.') { 
			switch (doMetaCommand(ib)) { 
				case (META_COMMAND_SUCCESS):
					continue;
				case (META_COMMAND_UNRECOGNIZED_COMMAND):
					printf("Unrecognized command '%s'\n", ib->buffer);
					continue;
			}
		}

		Statement statement;
		switch(prepareStatement(ib, &statement)) { 
			case (PREPARE_SUCCESS):
				break;
			case (PREPARE_UNRECOGNIZED_STATEMENT):
				printf("Unrecognized keyword at start of '%s'\n", ib->buffer);
				continue;
		}
		
		executeStatement(&statement);
		printf("Executed.\n");
	}
}

void printPrompt() { 
	printf("db > ");
}

MetaCommandResult doMetaCommand(InputBuffer ib) { 
	if (strcmp(ib->buffer, ".exit") == 0) { 
		exit(0);
	} else { 
		return META_COMMAND_UNRECOGNIZED_COMMAND;
	}
}

PrepareResult prepareStatement(InputBuffer ib, Statement * statement) { 
	if (strncmp(ib->buffer, "insert", 6) == 0) { 
		statement->type = STATEMENT_INSERT;
		return PREPARE_SUCCESS;
	}
	if (strcmp(ib->buffer, "select") == 0) { 
		statement->type = STATEMENT_SELECT;
		return PREPARE_SUCCESS;
	}
	return PREPARE_UNRECOGNIZED_STATEMENT;
}

void executeStatement(Statement * statement) { 
	switch (statement->type) { 
		case (STATEMENT_INSERT):
			printf("This is where we would do an insert.\n");
			break;
		case (STATEMENT_SELECT):
			printf("This is where we would do a select.\n");
			break;
	}
}
