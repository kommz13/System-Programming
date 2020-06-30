OBJS	= main.o red_black_tree.o listlist.o functions.o hashdisease.o hashcountry.o date.o PatientData.o
SOURCE	= main.c red_black_tree.c listlist.c functions.c hashdisease.c hashcountry.c date.c PatientData.c
HEADER	= red_black_tree.h listlist.h functions.h PatientData.h hashdisease.h hashcountry.h date.h
OUT	= diseaseMonitor
CC	 = gcc
FLAGS	 = -g -c -Wall
LFLAGS	 = 

all: $(OBJS)
	$(CC) -g $(OBJS) -o $(OUT) $(LFLAGS)

patientRecord: $(OBJS)
	$(CC) -g $(OBJS) -o $(OUT)
main.o: main.c
	$(CC) $(FLAGS) main.c 

red_black_tree.o: red_black_tree.c
	$(CC) $(FLAGS) red_black_tree.c 

listlist.o: listlist.c
	$(CC) $(FLAGS) listlist.c 

functions.o: functions.c
	$(CC) $(FLAGS) functions.c 

hashdisease.o: hashdisease.c
	$(CC) $(FLAGS) hashdisease.c

hashcountry.o: hashcountry.c
	$(CC) $(FLAGS) hashcountry.c

date.o: date.c
	$(CC) $(FLAGS) date.c

PatientData.o: PatientData.c
	$(CC) $(FLAGS) PatientData.c


clean:
	rm -f $(OBJS) $(OUT)