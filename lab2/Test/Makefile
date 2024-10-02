src = Test.c
obj = Test.o 
opt = -g -c

all: Test
Test: $(obj)
	gcc $(obj) -l pthread -o Test
Test.o: $(src)
	gcc $(opt) $(src) 
clean:
	rm Test *.o