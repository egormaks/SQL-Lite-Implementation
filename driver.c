#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>
#include "InputBuffer.h"
#include "Table.h"

typedef enum { EXECUTE_SUCCESS, 
	EXECUTE_TABLE_FULL 
} ExecuteResult;

typedef enum {
   META_COMMAND_SUCCESS,
   META_COMMAND_UNRECOGNIZED_COMMAND
} MetaCommandResult;

typedef enum {
  PREPARE_SUCCESS,
  PREPARE_SYNTAX_ERROR,
  PREPARE_UNRECOGNIZED_STATEMENT
 } PrepareResult;

typedef enum { 
	STATEMENT_INSERT, STATEMENT_SELECT 
} StatementType;

typedef struct {
  StatementType type;
  Row row_to_insert; //only used by insert statement
} Statement;

void printPrompt();
MetaCommandResult doMetaCommand(InputBuffer);
PrepareResult prepareStatement();
ExecuteResult executeStatement(Statement * statement, Table *);
ExecuteResult executeSelect(Statement *, Table *);
ExecuteResult executeInsert(Statement *, Table *);

int main(int argc, char* argv[]) {
	Table * table = newTable(); 
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
			case (PREPARE_SYNTAX_ERROR):
				printf("Syntax error. Could not parse statement.\n");
				continue;
			case (PREPARE_UNRECOGNIZED_STATEMENT):
				printf("Unrecognized keyword at start of '%s'\n", ib->buffer);
				continue;
		}
		
		switch(executeStatement(&statement, table)) {
			case (EXECUTE_SUCCESS):
				printf("Executed.\n");
				break;
			case (EXECUTE_TABLE_FULL):
				printf("Error: Table full.\n");
				break;
		}
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
		int argsAssigned = sscanf(
			ib->buffer, "insert %d %s %s", &(statement->row_to_insert.id),
			statement->row_to_insert.username, statement->row_to_insert.email);
		if (argsAssigned < 3) { 
			return PREPARE_SYNTAX_ERROR;
		}
		return PREPARE_SUCCESS;
	}
	if (strcmp(ib->buffer, "select") == 0) { 
		statement->type = STATEMENT_SELECT;
		return PREPARE_SUCCESS;
	}
	return PREPARE_UNRECOGNIZED_STATEMENT;
}

ExecuteResult executeStatement(Statement * statement, Table * table) { 
	switch (statement->type) { 
		case (STATEMENT_INSERT):
			return executeInsert(statement, table);
		case (STATEMENT_SELECT):
			return executeSelect(statement, table);
	}
}		

ExecuteResult executeInsert(Statement * statement, Table * table) { 
	if (canBeInsertedInto(table)) { 
		return EXECUTE_TABLE_FULL;
	}
	Row * row_to_insert = &(statement->row_to_insert);
	serializeRow(row_to_insert, rowSlot(table, table->num_rows));
	table->num_rows += 1;
	return EXECUTE_SUCCESS;	
}

ExecuteResult executeSelect(Statement * statement, Table * table) { 
	Row row;
	for (uint32_t i = 0; i < table->num_rows; i++) { 
		deserializeRow(rowSlot(table, i), &row);
		printRow(&row);
	}
	return EXECUTE_SUCCESS;
}


