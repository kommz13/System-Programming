OBJS	= main_server.o main_worker.o functions_server.o functions_worker.o extio.o queue.o listlist.o date.o
SOURCE	= main_server.c main_worker.c functions_server.c functions_worker.c extio.c queue.c listlist.c date.c
HEADER	= functions_server.h functions_worker.h queue.h listlist.h extio.h date.h Request.h
OUT	= whoServer
CC	 = gcc
FLAGS	 = -g -c -Wall
LFLAGS	 = -lpthread

all: $(OBJS)
	$(CC) -g $(OBJS) -o $(OUT) $(LFLAGS)

whoServer: $(OBJS)
	$(CC) -g $(OBJS) -o $(OUT) -lpthread

main_server.o: main_server.c
	$(CC) $(FLAGS) main_server.c -lpthread

main_worker.o: main_worker.c
	$(CC) $(FLAGS) main_worker.c -lpthread

functions_server.o: functions_server.c
	$(CC) $(FLAGS) functions_server.c -lpthread

functions_worker.o: functions_worker.c
	$(CC) $(FLAGS) functions_worker.c -lpthread

extio.o: extio.c
	$(CC) $(FLAGS) extio.c

queue.o: queue.c
	$(CC) $(FLAGS) queue.c 

listlist.o: listlist.c
	$(CC) $(FLAGS) listlist.c 

date.o: date.c
	$(CC) $(FLAGS) date.c

clean:
	rm -f $(OBJS) $(OUT)