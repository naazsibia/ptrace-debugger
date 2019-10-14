TESTS = multiple_siblings simple_fork Blank onePrint simple_pipe mixFork twoPipe

CC = @gcc
all: ${TESTS}
.PHONY: clean

%.o: %.c
	${CC} -c $< -o $@

clean:
	@rm -f *.o 
	@rm -f ${TESTS}