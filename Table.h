#ifndef TABLE_H_
#define TABLE_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define size_of_attribute(Struct, Attribute) sizeof(((Struct*)0)->Attribute)
#define TABLE_MAX_PAGES 100
#define COLUMN_USERNAME_SIZE 32
#define COLUMN_EMAIL_SIZE 255

typedef struct { 
    int file_descriptor;
    uint32_t file_length;
    void * pages[TABLE_MAX_PAGES];
    uint32_t num_pages;
} Pager;

typedef struct {
  uint32_t id;
  char username[COLUMN_USERNAME_SIZE + 1];
  char email[COLUMN_EMAIL_SIZE + 1];
} Row;

typedef struct {
  uint32_t root_page_num;
  Pager * pager;
  uint32_t num_rows;
} Table;

typedef struct  { 
  Table * table;
  bool end_of_table;
  uint32_t page_num;
  uint32_t cell_num;
} Cursor;

typedef enum { 
  NODE_INTERNAL,
  NODE_LEAF
} NodeType;

void serializeRow(Row * source, void * destination);
void deserializeRow(void * source, Row * destination);
void * cursorValue(Cursor * cursor);
void * getPage(Pager * pager, uint32_t page_num);
Table * dbOpen();
Pager * pagerOpen(const char * filename);
void dbClose(Table * table);
void pagerFlush(Pager * pager, uint32_t page_num);
void printRow(Row *);
int canBeInsertedInto(Table * table);
Cursor * tableStart(Table * table);
Cursor * tableEnd(Table * table);
void cursorAdvance(Cursor * cursor);
uint32_t * leafNodeNumCells(void * node);
uint32_t * leafNodeCell(void * node, uint32_t cell_num);
uint32_t * leafNodeKey(void * node, uint32_t cell_num);
uint32_t * leafNodeValue(void * node, uint32_t cell_num);
void initializeLeafNode(void * node);
void leafNodeInsert(Cursor * cursor, uint32_t key, Row * value);

#endif
