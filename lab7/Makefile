src = Pmap.cpp
obj = Pmap.o 
opt = -w -g -c

all: Pmap
Pmap: $(obj)
	g++ $(obj) -o Pmap
Pmap.o: $(src)
	g++ $(opt) $(src) 
clean:
	rm Pmap *.o