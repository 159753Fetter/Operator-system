hdrs = ipc.h
opts = -g -c

p_src = producer.c ipc.c 
p_obj = producer.o ipc.o

smoker_A_src = smoker_A.c ipc.c 
smoker_A_obj = smoker_A.o ipc.o

smoker_B_src = smoker_B.c ipc.c 
smoker_B_obj = smoker_B.o ipc.o

smoker_C_src = smoker_C.c ipc.c 
smoker_C_obj = smoker_C.o ipc.o

all:	producer smoker_A smoker_B smoker_C

producer: $(p_obj)
	gcc $(p_obj) -o producer
producer.o: $(p_src) $(hdrs)
	gcc $(opts) $(p_src) 

smoker_A: $(smoker_A_obj)
	gcc $(smoker_A_obj) -o smoker_A
smoker_A.o: $(smoker_A_src) $(hdrs)
	gcc $(opts) $(smoker_A_src) 

smoker_B: $(smoker_B_obj)
	gcc $(smoker_B_obj) -o smoker_B
smoker_B.o: $(smoker_B_src) $(hdrs)
	gcc $(opts) $(smoker_B_src) 

smoker_C: $(smoker_C_obj)
	gcc $(smoker_C_obj) -o smoker_C
smoker_C.o: $(smoker_C_src) $(hdrs)
	gcc $(opts) $(smoker_C_src) 

clean:
	rm producer smoker_A smoker_B smoker_C *.o