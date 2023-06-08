CC		= gcc
CFLAGS	= -Wall -std=c99
LIBS	= -lpthread
HFILES	= $(wildcard *.h)
CFILES	= $(wildcard *.c)
OBJS	= $(patsubst %.c, %.o, ${CFILES})

.PHONY: all clean

all: assembly

assembly: ${OBJS}
	${CC} ${CFLAGS} $^ ${LIBS} -o $@ 

%.o: %.c ${HFILES}
	${CC} ${CFLAGS} -c $< -o $@

clean:
	-@rm -rf ${OBJS} assembly 
