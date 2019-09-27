SUBDIRS = Tests
OBJ = pdt.o avl_tree.o dead_ll.o
DEPENDENCIES = pdt.h 
.PHONY: ${SUBDIRS} clean

all: start  ${SUBDIRS}

start: ${OBJ}
	@gcc -o pdt ${OBJ}

${SUBDIRS}:
	@make -C $@

%.o: %.c ${DEPENDENCIES}
	@gcc -c $<

clean:
	@rm -f *.o ${OBJ}
	@for subd in ${SUBDIRS}; do \
       make -C $${subd} clean; \
    done