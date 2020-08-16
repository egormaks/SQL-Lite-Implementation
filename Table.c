#include "Table.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

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
	void * page = getPage(table->pager, page_num);
	uint32_t row_offset = row_num % ROWS_PER_PAGE;
	uint32_t byte_offset = row_offset * ROW_SIZE;
	return page + byte_offset;
}

void * getPage(Pager * pager, uint32_t pageNum) { 
    if (pageNum > TABLE_MAX_PAGES) { 
        printf("Tried to reach page that is out of bounds. %d > %d.\n", pageNum, TABLE_MAX_PAGES);
    }
    if (pager->pages[pageNum] == NULL) { 
        void * page = malloc(PAGE_SIZE);
        uint32_t numPages = pager->file_length / PAGE_SIZE;
        if (pager->file_length % PAGE_SIZE) {
            numPages += 1;
        }
        if (pageNum <= numPages) { 
            lseek(pager->file_descriptor, pageNum * PAGE_SIZE, SEEK_SET);
            ssize_t bytesRead = read(pager->file_descriptor, page, PAGE_SIZE);
            if (bytesRead == -1) { 
                printf("Failed to read file.\n");
                exit(EXIT_FAILURE);
            }
        }
        pager->pages[pageNum] = page;
    }
    return pager->pages[pageNum];
}

Table * dbOpen(const char * filename) { 
	Table * table = malloc(sizeof(Table));
    table->pager = pagerOpen(filename);
    table->num_rows = table->pager->file_length / ROW_SIZE;
	return table;
}

Pager * pagerOpen(const char * filename) { 
    int fd = open(filename, O_RDWR | O_CREAT, S_IWUSR | S_IRUSR);
    if (fd == -1) {
        printf("Unable to open file.\n");
        exit(EXIT_FAILURE);
    }
    off_t fileLen = lseek(fd, 0, SEEK_END);
    Pager * pager = malloc(sizeof(Pager));
    pager->file_descriptor = fd;
    pager->file_length = fileLen;
    for (uint32_t i = 0; i < TABLE_MAX_PAGES; i++) { 
        pager->pages[i] = NULL;
    }
    return pager;
}

void dbClose(Table * table) { 
	Pager * pager = table->pager;
    uint32_t numFullPages = table->num_rows / ROWS_PER_PAGE;
    uint32_t i;
    for (i = 0; i < numFullPages; i++) { 
        if (pager->pages[i] == NULL)
            continue;
        pagerFlush(pager, i, PAGE_SIZE);
        free(pager->pages[i]);
        pager->pages[i] = NULL;
    }

    uint32_t numAdditionalRows = table->num_rows % ROWS_PER_PAGE;
    if (numAdditionalRows > 0) { 
        uint32_t page_num = numFullPages;
        if (pager->pages[page_num] != NULL) { 
            pagerFlush(pager, page_num, numAdditionalRows * ROW_SIZE);
            free(pager->pages[page_num]);
            pager->pages[page_num] = NULL;
        }
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

void pagerFlush(Pager * pager, uint32_t pageNum, uint32_t size) {
    if (pager->pages[pageNum] == NULL) { 
        printf("Tried to flush empty page.\n");
        exit(EXIT_FAILURE);
    }
    off_t offset = lseek(pager->file_descriptor, pageNum * PAGE_SIZE, SEEK_SET);
    if (offset == -1) { 
        printf("Error seeking: %d\n", errno);
        exit(EXIT_FAILURE);
    }
    ssize_t bytesWritten = write(pager->file_descriptor, pager->pages[pageNum], size);
    if (bytesWritten == -1) { 
        printf("Error writing: %d\n", errno);
        exit(EXIT_FAILURE);
    }
}

void printRow(Row * row) { 
	printf("(%d, %s, %s)\n", row->id, row->username, row->email);
}

int canBeInsertedInto(Table * table) {
    return table->num_rows >= TABLE_MAX_ROWS;
}
