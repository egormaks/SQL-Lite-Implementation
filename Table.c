#include "Table.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <stdbool.h>

const uint32_t ID_SIZE = size_of_attribute(Row, id);
const uint32_t USERNAME_SIZE = size_of_attribute(Row, username);
const uint32_t EMAIL_SIZE = size_of_attribute(Row, email);
const uint32_t ROW_SIZE = ID_SIZE + USERNAME_SIZE + EMAIL_SIZE;

const uint32_t ID_OFFSET = 0;
const uint32_t USERNAME_OFFSET = ID_OFFSET + ID_SIZE;
const uint32_t EMAIL_OFFSET = USERNAME_OFFSET + USERNAME_SIZE;

const uint32_t PAGE_SIZE = 4096;

const uint32_t NODE_TYPE_SIZE = sizeof(uint8_t);
const uint32_t NODE_TYPE_OFFSET = 0;
const uint32_t IS_ROOT_SIZE = sizeof(uint8_t);
const uint32_t IS_ROOT_OFFSET = NODE_TYPE_SIZE;
const uint32_t PARENT_POINTER_SIZE = sizeof(uint32_t);
const uint32_t PARENT_POINTER_OFFSET = IS_ROOT_SIZE + IS_ROOT_OFFSET;
const uint8_t  COMMON_NODE_HEADER_SIZE = NODE_TYPE_SIZE + IS_ROOT_SIZE + PARENT_POINTER_SIZE;

const uint32_t LEAF_NODE_NUM_CELLS_SIZE = sizeof(uint32_t);
const uint32_t LEAF_NODE_NUM_CELLS_OFFSET = COMMON_NODE_HEADER_SIZE;
const uint32_t LEAF_NODE_HEADER_SIZE = COMMON_NODE_HEADER_SIZE + LEAF_NODE_NUM_CELLS_SIZE;

const uint32_t LEAF_NODE_KEY_SIZE = sizeof(uint32_t);
const uint32_t LEAF_NODE_KEY_OFFSET = 0;
const uint32_t LEAF_NODE_VALUE_SIZE = ROW_SIZE;
const uint32_t LEAF_NODE_VALUE_OFFSET = LEAF_NODE_KEY_OFFSET + LEAF_NODE_KEY_SIZE;
const uint32_t LEAF_NODE_CELL_SIZE = LEAF_NODE_KEY_SIZE + LEAF_NODE_VALUE_SIZE;
const uint32_t LEAF_NODE_SPACE_FOR_CELLS = PAGE_SIZE - LEAF_NODE_HEADER_SIZE;
const uint32_t LEAF_NODE_MAX_CELLS = LEAF_NODE_SPACE_FOR_CELLS / LEAF_NODE_CELL_SIZE;

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

void * getPage(Pager * pager, uint32_t page_num) { 
    if (page_num > TABLE_MAX_PAGES) { 
        printf("Tried to reach page that is out of bounds. %d > %d.\n", page_num, TABLE_MAX_PAGES);
    }
    if (pager->pages[page_num] == NULL) { 
        void * page = malloc(PAGE_SIZE);
        uint32_t num_pages = pager->file_length / PAGE_SIZE;
        if (pager->file_length % PAGE_SIZE) {
            num_pages += 1;
        }
        if (page_num <= num_pages) { 
            lseek(pager->file_descriptor, page_num * PAGE_SIZE, SEEK_SET);
            ssize_t bytesRead = read(pager->file_descriptor, page, PAGE_SIZE);
            if (bytesRead == -1) { 
                printf("Failed to read file.\n");
                exit(EXIT_FAILURE);
            }
        }
        pager->pages[page_num] = page;
        if (page_num >= pager->num_pages) { 
            pager->num_pages += 1;
        }
    }
    return pager->pages[page_num];
}

Table * dbOpen(const char * filename) { 
	Table * table = malloc(sizeof(Table));
    table->pager = pagerOpen(filename);
    table->root_page_num = 0;
    if (table->pager->num_pages == 0) { 
        void * root_node = getPage(table->pager, 0);
        initializeLeafNode(root_node);
    }
	return table;
}

Pager * pagerOpen(const char * filename) { 
    int fd = open(filename, O_RDWR | O_CREAT, S_IWUSR | S_IRUSR);
    if (fd == -1) {
        printf("Unable to open file.\n");
        exit(EXIT_FAILURE);
    }
    off_t file_length = lseek(fd, 0, SEEK_END);
    Pager * pager = malloc(sizeof(Pager));
    pager->file_descriptor = fd;
    pager->file_length = file_length;
    pager->num_pages = (file_length / PAGE_SIZE);

    if (file_length % PAGE_SIZE != 0) { 
        printf("Db file does not contain whole number of pages. Corrupt file. Exiting.");
        exit(EXIT_FAILURE);
    }

    for (uint32_t i = 0; i < TABLE_MAX_PAGES; i++) { 
        pager->pages[i] = NULL;
    }
    return pager;
}

void dbClose(Table * table) { 
	Pager * pager = table->pager;
    uint32_t i;
    for (i = 0; i < pager->num_pages; i++) { 
        if (pager->pages[i] == NULL)
            continue;
        pagerFlush(pager, i);
        free(pager->pages[i]);
        pager->pages[i] = NULL;
    }

    int result = close(pager->file_descriptor);
    if (result == -1) { 
        printf("Error closing db file.\n");
        exit(EXIT_FAILURE);
    }
    for (uint32_t i = 0; i < TABLE_MAX_PAGES; i++) { 
        void * page = pager->pages[i];
        if (page) { 
            free(page);
            pager->pages[i] = NULL;
        }
    }
    
    free(pager);
    free(table);
}

void pagerFlush(Pager * pager, uint32_t page_num) {
    if (pager->pages[page_num] == NULL) { 
        printf("Tried to flush empty page.\n");
        exit(EXIT_FAILURE);
    }
    off_t offset = lseek(pager->file_descriptor, page_num * PAGE_SIZE, SEEK_SET);
    if (offset == -1) { 
        printf("Error seeking: %d\n", errno);
        exit(EXIT_FAILURE);
    }
    ssize_t bytesWritten = write(pager->file_descriptor, pager->pages[page_num], PAGE_SIZE);
    if (bytesWritten == -1) { 
        printf("Error writing: %d\n", errno);
        exit(EXIT_FAILURE);
    }
}

void printRow(Row * row) { 
	printf("(%d, %s, %s)\n", row->id, row->username, row->email);
}

Cursor * tableStart(Table * table) { 
    Cursor * cursor = malloc(sizeof(Cursor));
    cursor->table = table;
    cursor->page_num = table->root_page_num;
    cursor->cell_num = 0;

    void * root_node = getPage(table->pager, table->root_page_num);
    uint32_t num_cells = *leafNodeNumCells(root_node);
    cursor->end_of_table = (num_cells == 0);
    return cursor;
}

Cursor * tableFind(Table * table, uint32_t key) { 
    uint32_t root_page_num = table->root_page_num;
    void * root_node = getPage(table->pager, table->root_page_num);
    if (getNodeType(root_node) == NODE_LEAF) { 
        return leafNodeFind(table, root_page_num, key);
    } else { 
        printf("Cannot search internal node yet.");
        exit(EXIT_FAILURE);
    }
}

void cursorAdvance(Cursor * cursor) {  
  uint32_t page_num = cursor->page_num;
  void * node = getPage(cursor->table->pager, page_num);
  cursor->cell_num += 1;
  if (cursor->cell_num >= (*leafNodeNumCells(node)))
    cursor->end_of_table = true;
}

void * cursorValue(Cursor * cursor) {
    uint32_t page_num = cursor->page_num;
	void * page = getPage(cursor->table->pager, page_num);
	return leafNodeValue(page, cursor->cell_num);
}

uint32_t * leafNodeNumCells(void * node) { 
    return node + LEAF_NODE_NUM_CELLS_OFFSET;
}

uint32_t * leafNodeCell(void * node, uint32_t cell_num) {
    return node + LEAF_NODE_HEADER_SIZE + cell_num * LEAF_NODE_CELL_SIZE;
}

uint32_t * leafNodeKey(void * node, uint32_t cell_num) {
    return leafNodeCell(node, cell_num);
}

uint32_t * leafNodeValue(void * node, uint32_t cell_num){ 
    return leafNodeCell(node, cell_num) + LEAF_NODE_KEY_SIZE;
}

void initializeLeafNode(void * node) {
    setNodeType(node, NODE_LEAF);
    *leafNodeNumCells(node) = 0;
}

void leafNodeInsert(Cursor * cursor, uint32_t key, Row * value) { 
    void * node = getPage(cursor->table->pager, cursor->page_num);
    uint32_t num_cells = *leafNodeNumCells(node);
    if (num_cells >= LEAF_NODE_MAX_CELLS) { 
        printf("Implement leaf node split");
        exit(EXIT_FAILURE);
    }
    if (cursor->cell_num < num_cells) { 
        for (uint32_t i = num_cells; i > cursor->cell_num; i--) { 
            memcpy(leafNodeCell(node, i), leafNodeCell(node, i - 1), LEAF_NODE_CELL_SIZE);
        }
    }

    *(leafNodeNumCells(node)) += 1;
    *(leafNodeKey(node, cursor->cell_num)) = key;
    serializeRow(value, leafNodeValue(node, cursor->cell_num));
}

NodeType getNodeType(void * node) { 
    uint8_t val = *((uint8_t*)(node + NODE_TYPE_OFFSET));
    return (NodeType) val;
}
void setNodeType(void * node, NodeType type) { 
    uint8_t val = type;
    *((uint8_t*)(node + NODE_TYPE_OFFSET)) = val;
}

Cursor * leafNodeFind(Table * table, uint32_t page_num, uint32_t key) { 
    void * node = getPage(table->pager, page_num);
    Cursor * cursor = malloc(sizeof(Cursor));
    uint32_t num_cells = *leafNodeNumCells(node);
    cursor->table = table;
    cursor->page_num = page_num;
    uint32_t min_index = 0;
    uint32_t past_max = num_cells;
    while (past_max != min_index) { 
        uint32_t index = (min_index + past_max) / 2;
        uint32_t key_at_index = *leafNodeKey(node, index);
        if (key == key_at_index) { 
            cursor->cell_num = index;
            return cursor;
        } else if (key < key_at_index) { 
            past_max = index;
        } else { 
            min_index = index + 1;
        }
    }
    cursor->cell_num = min_index;
    return cursor;
}
