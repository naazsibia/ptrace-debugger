SUBDIRS = Tests
OBJ = pdt.o avl_tree.o dead_ll.o log.o
DEPENDENCIES = pdt.h avl_tree.h dead_ll.h log.h
FLAGS = -fsanitize=address -fsanitize=undefined 
.PHONY: ${SUBDIRS} clean

all: start  ${SUBDIRS}

start: ${OBJ}
	@gcc -o pdt ${OBJ} ${FLAGS}

${SUBDIRS}:
	@make -C $@

%.o: %.c ${DEPENDENCIES}
	@gcc -c $< ${FLAGS}

clean:
	@rm -f *.o ${OBJ}
	@for subd in ${SUBDIRS}; do \
       make -C $${subd} clean; \
    done
