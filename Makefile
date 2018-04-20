
OBJ = rados.o
CC?=gcc
CFLAGS?=-O2 -g -Wall

.c.o:
	gcc ${CFLAGS} -c $< -o $@

rados: ${OBJ}
	gcc -o rados ${OBJ} -lrados

all: rados

clean:
	rm -f ${OBJ}
	rm -f rados

realclean: clean
	rm -f *~

