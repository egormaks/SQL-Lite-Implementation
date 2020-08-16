#include "Table.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const uint32_t ID_SIZE = size_of_attribute(Row, id);
const uint32_t USERNAME_SIZE = size_of_attribute(Row, username);
const uint32_t EMAIL_SIZE = size_of_attribute(Row, email);
const uint32_t ROW_SIZE = ID_SIZE + USERNAME_SIZE + EMAIL_SIZE;

const uint32_t ID_OFFSET = 0;
const uint32_t USERNAME_OFFSET = ID_OFFSET + ID_SIZE;
const uint32_t EMAIL_OFFSET = USERNAME_OFFSET + USERNAME_SIZE;

const uint32_t PAGE_SIZE = 4096;
const uint32_t ROWS_PER_PAGE = PAGE_SIZE / ROW_SIZE;
const uint32_t TABLE_MAX_ROWS = ROWS_PER_PAGE * TABLE_MAX_PAGES;

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

int canBeInsertedInto(Table * table) {
    return table->num_rows >= TABLE_MAX_ROWS;
}
