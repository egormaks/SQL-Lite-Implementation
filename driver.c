 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 #include <time.h>
#include <stdint.h>
#include "InputBuffer.h"

#define size_of_attribute(Struct, Attribute) sizeof(((Struct*)0)->Attribute)
#define TABLE_MAX_PAGES 100
#define COLUMN_USERNAME_SIZE 32
#define COLUMN_EMAIL_SIZE 255

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
  uint32_t id;
  char username[COLUMN_USERNAME_SIZE];
  char email[COLUMN_EMAIL_SIZE];
} Row;

typedef struct {
  StatementType type;
  Row row_to_insert; //only used by insert statement
} Statement;

typedef struct {
  uint32_t num_rows;
  void* pages[TABLE_MAX_PAGES];
} Table;

const uint32_t ID_SIZE = size_of_attribute(Row, id);
const uint32_t USERNAME_SIZE = size_of_attribute(Row, username);
const uint32_t EMAIL_SIZE = size_of_attribute(Row, email);
const uint32_t ID_OFFSET = 0;
const uint32_t USERNAME_OFFSET = ID_OFFSET + ID_SIZE;
const uint32_t EMAIL_OFFSET = USERNAME_OFFSET + USERNAME_SIZE;
const uint32_t ROW_SIZE = ID_SIZE + USERNAME_SIZE + EMAIL_SIZE;

const uint32_t PAGE_SIZE = 4096;
const uint32_t ROWS_PER_PAGE = PAGE_SIZE / ROW_SIZE;
const uint32_t TABLE_MAX_ROWS = ROWS_PER_PAGE * TABLE_MAX_PAGES;

void printPrompt();
MetaCommandResult doMetaCommand(InputBuffer);
PrepareResult prepareStatement();
ExecuteResult executeStatement(Statement * statement, Table *);
ExecuteResult executeSelect(Statement *, Table *);
ExecuteResult executeInsert(Statement *, Table *);
void serializeRow(Row * source, void * destination);
void deserializeRow(void * source, Row * destination);
void * rowSlot(Table * table, uint32_t row_num);
Table * newTable();
void freeTable(Table * table);
void printRow(Row *);

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
	if (table->num_rows >= TABLE_MAX_ROWS) { 
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

void serializeRow(Row * source, void * destination) { 
	memcpy(destination + ID_OFFSET, &(source->id), ID_SIZE);
	memcpy(destination + USERNAME_OFFSET, &(source->username), USERNAME_SIZE);
	memcpy(destination + EMAIL_OFFSET, &(source->email), EMAIL_SIZE);
}

void deserializeRow(void * source, Row * destination) { 
	memcpy(&(destination->id), source + ID_OFFSET, ID_SIZE);
	memcpy(&(destination->username), source + USERNAME_OFFSET, USERNAME_SIZE);
	memcpy(&(destination->email), source + EMAIL_OFFSET, EMAIL_SIZE);
}

void * rowSlot(Table * table, uint32_t row_num) {
	uint32_t page_num = row_num / ROWS_PER_PAGE;
	void * page = table->pages[page_num];
	if (page == NULL) { 
		page = table->pages[page_num] = malloc(PAGE_SIZE);
	}
	uint32_t row_offset = row_num % ROWS_PER_PAGE;
	uint32_t byte_offset = row_offset * ROW_SIZE;
	return page + byte_offset;
}


Table * newTable() { 
	Table * table = malloc(sizeof(Table));
	table->num_rows = 0;
	for (uint32_t i = 0; i < TABLE_MAX_PAGES; i++) { 
		table->pages[i] = NULL;
	}
	return table;
}

void freeTable(Table * table) { 
	for (int i = 0; table->pages[i]; i++) { 
		free(table->pages[i]);
	}
	free(table);
}

void printRow(Row * row) { 
	printf("(%d, %s, %s)\n", row->id, row->username, row->email);
}
