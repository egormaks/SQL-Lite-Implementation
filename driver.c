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
  PREPARE_UNRECOGNIZED_STATEMENT,
  PREPARE_STRING_TOO_LONG,
  PREPARE_NEGATIVE_ID
 } PrepareResult;

typedef enum { 
	STATEMENT_INSERT, STATEMENT_SELECT 
} StatementType;

typedef struct {
  StatementType type;
  Row row_to_insert; //only used by insert statement
} Statement;

extern uint32_t LEAF_NODE_MAX_CELLS;

void printPrompt();
MetaCommandResult doMetaCommand(InputBuffer, Table *);
PrepareResult prepareStatement();
ExecuteResult executeStatement(Statement * statement, Table *);
ExecuteResult executeSelect(Statement *, Table *);
ExecuteResult executeInsert(Statement *, Table *);

int main(int argc, char* argv[]) {
	if (argc < 2) { 
		printf("Must supply a database filename.\n");
		exit(EXIT_FAILURE);
	}
	char * filename = argv[1];
	Table * table = dbOpen(filename); 
	InputBuffer ib = createInputBuffer();
	time_t t;
	time(&t);
	printf("Egor's SQLite version 1.0 %s\n", ctime(&t));
	printf("Enter \".help\" for usage hints.\n");
	while (1) { 
		printPrompt();
		readInput(ib);

		if (ib->buffer[0] == '.') { 
			switch (doMetaCommand(ib, table)) { 
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
			case (PREPARE_STRING_TOO_LONG):
				printf("String is too long.\n");
				continue;
			case (PREPARE_NEGATIVE_ID):
				printf("Negative ids are forbidden.\n");
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

MetaCommandResult doMetaCommand(InputBuffer ib, Table * table) { 
	if (strcmp(ib->buffer, ".exit") == 0) { 
		deleteInputBuffer(ib);
		dbClose(table);
		exit(0);
	} else { 
		return META_COMMAND_UNRECOGNIZED_COMMAND;
	}
}

PrepareResult prepareStatement(InputBuffer ib, Statement * statement) { 
	if (strncmp(ib->buffer, "insert", 6) == 0) { 
		statement->type = STATEMENT_INSERT;
		char * keyword = strtok(ib->buffer, " ");
		char * idString = strtok(NULL, " ");
		char * username = strtok(NULL, " ");
		char * email = strtok(NULL, " ");
		int id = atoi(idString);
		if (idString == NULL || username == NULL || email == NULL) 
			return PREPARE_SYNTAX_ERROR;
		if (strlen(username) > COLUMN_USERNAME_SIZE || strlen(email) > COLUMN_EMAIL_SIZE) 
			return PREPARE_STRING_TOO_LONG;
		if (id < 0)
			return PREPARE_NEGATIVE_ID;

		statement->row_to_insert.id = id;
		strcpy(statement->row_to_insert.username, username);
		strcpy(statement->row_to_insert.email, email);

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
	void * node = getPage(table->pager, table->root_page_num);
	if (*leafNodeNumCells(node) >= LEAF_NODE_MAX_CELLS) { 
		return EXECUTE_TABLE_FULL;
	}
	Row * row_to_insert = &(statement->row_to_insert);
	Cursor * cursor = tableEnd(table);
	leafNodeInsert(cursor, row_to_insert->id, row_to_insert);
	free(cursor);
	return EXECUTE_SUCCESS;	
}

ExecuteResult executeSelect(Statement * statement, Table * table) { 
	Cursor * cursor = tableStart(table);
	Row row;
	while (!(cursor->end_of_table)) { 
		deserializeRow(cursorValue(cursor), &row);
		printRow(&row);
		cursorAdvance(cursor);
	}
	free(cursor);
	return EXECUTE_SUCCESS;
}


