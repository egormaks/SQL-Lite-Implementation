#ifndef TABLE_H_
#define TABLE_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define size_of_attribute(Struct, Attribute) sizeof(((Struct*)0)->Attribute)
#define TABLE_MAX_PAGES 100
#define COLUMN_USERNAME_SIZE 32
#define COLUMN_EMAIL_SIZE 255

// const uint32_t NODE_TYPE_SIZE = sizeof(uint8_t);
// const uint32_t NODE_TYPE_OFFSET = 0;
// const uint32_t IS_ROOT_SIZE = sizeof(uint8_t);
// const uint32_t IS_ROOT_OFFSET = NODE_TYPE_SIZE;
// const uint32_t PARENT_POINTER_SIZE = sizeof(uint32_t);
// const uint32_t PARENT_POINTER_OFFSET = IS_ROOT_SIZE + IS_ROOT_OFFSET;
// const uint8_t  COMMON_NODE_HEADER_SIZE = NODE_TYPE_SIZE + IS_ROOT_SIZE + PARENT_POINTER_SIZE;

// const uint32_t LEAF_NODE_NUM_CELLS_SIZE = sizeof(uint32_t);
// const uint32_t LEAF_NODE_NUM_CELLS_OFFSET = COMMON_NODE_HEADER_SIZE;
// const uint32_t LEAF_NODE_HEADER_SIZE = COMMON_NODE_HEADER_SIZE + LEAF_NODE_NUM_CELLS_SIZE;

typedef enum { 
  NODE_INTERNAL,
  NODE_LEAF
} NodeType;

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

typedef struct  { 
  Table * table;
  uint32_t row_num;
  bool end_of_table;
} Cursor;

void serializeRow(Row * source, void * destination);
void deserializeRow(void * source, Row * destination);
void * cursorValue(Cursor * cursor);
void * getPage(Pager * pager, uint32_t page_num);
Table * dbOpen();
Pager * pagerOpen(const char * filename);
void dbClose(Table * table);
void pagerFlush(Pager * pager, uint32_t page_num, uint32_t size);
void printRow(Row *);
int canBeInsertedInto(Table * table);
Cursor * tableStart(Table * table);
Cursor * tableEnd(Table * table);
void cursorAdvance(Cursor * cursor);
#endif
