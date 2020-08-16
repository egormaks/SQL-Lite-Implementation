CC=gcc
CFLAGS=-I.
DEPS = Table.h InputBuffer.h
OBJ = Table.c InputBuffer.c Driver.c 

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

hellomake: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)
