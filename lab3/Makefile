src = Test.c
obj = Test.o 
opt = -g -c

all: Test
Test: $(obj)
	gcc $(obj) -o Test -lreadline
Test.o: $(src)
	gcc $(opt) $(src) 
clean:
	rm Test *.o