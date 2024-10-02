head = test.h
srcs = test.c
objs = test.o
opts = -g -c
all: test
test: $(objs)
	gcc $(objs) -o test 
test.o: $(srcs) $(head)
	gcc $(opts) $(srcs) 
clean:
	rm test *.o