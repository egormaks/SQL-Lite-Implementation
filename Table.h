#ifndef TABLE_H_
#define TABLE_H_

#include <stdio.h>
#include <stdlib.h>

#define size_of_attribute(Struct, Attribute) sizeof(((Struct*)0)->Attribute)
#define TABLE_MAX_PAGES 100
#define COLUMN_USERNAME_SIZE 32
#define COLUMN_EMAIL_SIZE 255

typedef struct { 
    int file_descriptor;
    uint32_t file_length;
    void * pages[TABLE_MAX_PAGES];
} Pager;

typedef struct {
  uint32_t id;
  char username[COLUMN_USERNAME_SIZE + 1];
  char email[COLUMN_EMAIL_SIZE + 1];
} Row;

typedef struct {
  uint32_t num_rows;
  Pager * pager;
} Table;

void serializeRow(Row * source, void * destination);
void deserializeRow(void * source, Row * destination);
void * rowSlot(Table * table, uint32_t row_num);
void * getPage(Pager * pager, uint32_t page_num);
Table * dbOpen();
Pager * pagerOpen(const char * filename);
void dbClose(Table * table);
void pagerFlush(Pager * pager, uint32_t pageNum, uint32_t size);
void printRow(Row *);
int canBeInsertedInto(Table * table);

#endif
