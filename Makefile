all : test
.PHONY : all
test : cpuinfo.c timer.c transpose.c test.c defs.h cpuinfo.h timer.h transpose.h test.h
	cc -o test cpuinfo.c timer.c transpose.c test.c -lpthread -O3 -std=c11

.PHONY : clean
clean :
	-rm test
