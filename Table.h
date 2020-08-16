#ifndef TABLE_H_
#define TABLE_H_

#include <stdio.h>
#include <stdlib.h>

#define size_of_attribute(Struct, Attribute) sizeof(((Struct*)0)->Attribute)
#define TABLE_MAX_PAGES 100
#define COLUMN_USERNAME_SIZE 32
#define COLUMN_EMAIL_SIZE 255

typedef struct {
  uint32_t id;
  char username[COLUMN_USERNAME_SIZE];
  char email[COLUMN_EMAIL_SIZE];
} Row;

typedef struct {
  uint32_t num_rows;
  void* pages[TABLE_MAX_PAGES];
} Table;

void serializeRow(Row * source, void * destination);
void deserializeRow(void * source, Row * destination);
void * rowSlot(Table * table, uint32_t row_num);
Table * newTable();
void freeTable(Table * table);
void printRow(Row *);
int canBeInsertedInto(Table * table);

#endif
