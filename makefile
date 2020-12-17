all: Trinity

main.o: main.c commands.h utilities.h
	gcc main.c -c -Wall -Wextra -lpthread

utilities.o: utilities.c utilities.h
	gcc utilities.c -c -Wall -Wextra -lpthread

commands.o: commands.c commands.h utilities.h
	gcc commands.c -c -Wall -Wextra -lpthread

llistaPDI.o: llistaPDI.c llistaPDI.h utilities.h
	gcc llistaPDI.c -c -Wall -Wextra -lpthread

connections.o: connections.c connections.h utilities.h llistaPDI.h protocol.h
	gcc connections.c -c -Wall -Wextra -lpthread

protocol.o: protocol.c protocol.h connections.h utilities.h
	gcc protocol.c -c -Wall -Wextra -lpthread

semaphore_v66.o: semaphore_v66.c semaphore_v66.h
	gcc semaphore_v66.c -c -Wall -Wextra -lpthread

Trinity: main.o commands.o utilities.o connections.o llistaPDI.o protocol.o semaphore_v66.o
	gcc main.o commands.o utilities.o connections.o llistaPDI.o protocol.o semaphore_v66.o -o Trinity -Wall -Wextra -lpthread

clean:
	rm *.o Trinity
