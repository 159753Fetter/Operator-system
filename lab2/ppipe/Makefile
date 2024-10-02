srcs = ppipe.c
objs = ppipe.o 
opts = -g -c

all: ppipe
tpipe: $(objs)
	gcc $(objs) -o ppipe
tpipe.o: $(srcs)
	gcc $(opts) $(srcs) 
clean:
	rm ppipe *.o