TESTS = multiple_siblings simple_fork
CC = @gcc
all: ${TESTS}
.PHONY: clean

%.o: %.c
	${CC} -c $< -o $@

clean:
	@rm -f *.o 
	@rm -f ${TESTS}