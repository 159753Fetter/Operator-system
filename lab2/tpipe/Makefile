src = tpipe.c
obj = tpipe.o 
opt = -g -c

all: tpipe
tpipe: $(obj)
	gcc $(obj) -l pthread -o tpipe
tpipe.o: $(src)
	gcc $(opt) $(src) 
clean:
	rm tpipe *.o