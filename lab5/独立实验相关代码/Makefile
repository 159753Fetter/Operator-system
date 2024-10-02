hdrs = ipc.h
opts = -g -c

b_src = barber.c ipc.c 
b_obj = barber.o ipc.o

c_src = customer.c ipc.c 
c_obj = customer.o ipc.o

all:	barber customer

barber: $(b_obj)
	gcc $(b_obj) -o barber
barber.o: $(b_src) $(hdrs)
	gcc $(opts) $(b_src) 

customer: $(c_obj)
	gcc $(c_obj) -o customer
customer.o: $(c_src) $(hdrs)
	gcc $(opts) $(c_src) 

clean:
	rm customer barber *.o