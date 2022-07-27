
CFLAGS=-g -Wall


program=project1 


objects=child.o parent.o main.o


$(program) : $(objects)
	gcc $(objects) -o $(program) -lpthread


main.o : parent.h child.h shared_memory.h
parent.o : parent.h shared_memory.h
child.o : child.h shared_memory.h


clean:
	rm $(program) $(objects)


run: $(program)
	./$(program)

