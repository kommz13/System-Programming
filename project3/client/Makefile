OBJS	= main_client.o main_worker.o functions_client.o extio.o date.o
SOURCE	= main_client.c main_worker.c functions_client.c extio.c date.c
HEADER	= functions_client.h extio.h PatientData.h date.h Request.h
OUT	= whoClient
CC = gcc
FLAGS =-g -c -Wall
LFLAGS =-lpthread

all: $(OBJS)
	$(CC) -g $(OBJS) -o $(OUT) $(LFLAGS)

whoClient: $(OBJS)
	$(CC) -g $(OBJS) -o $(OUT) -lpthread

main_client.o: main_client.c
	$(CC) $(FLAGS) main_client.c -lpthread

main_worker.o: main_worker.c
	$(CC) $(FLAGS) main_worker.c -lpthread

functions_client.o: functions_client.c
	$(CC) $(FLAGS) functions_client.c -lpthread

extio.o: extio.c
	$(CC) $(FLAGS) extio.c

date.o: date.c
	$(CC) $(FLAGS) date.c

clean:
	rm -f $(OBJS) $(OUT)