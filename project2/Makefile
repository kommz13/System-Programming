OBJS	= main_aggregator.o main_worker.o functions_aggregator.o functions_worker.o extio.o quicksort_top.o quicksort.o red_black_tree.o listlist.o hashdisease.o hashcountry.o date.o PatientData.o
SOURCE	= main_aggregator.c main_worker.c functions_aggregator.c functions_worker.c extio.c quicksort_top.c quicksort.c red_black_tree.c listlist.c hashdisease.c hashcountry.c date.c PatientData.c
HEADER	= red_black_tree.h listlist.h functions_aggregator.h functions_worker.h extio.h quicksort_top.h quicksort.h PatientData.h hashdisease.h hashcountry.h date.h Request.h
OUT	= diseaseAggregator
CC	 = gcc
FLAGS	 = -g -c -Wall
LFLAGS	 = 

all: $(OBJS)
	$(CC) -g $(OBJS) -o $(OUT) $(LFLAGS)

diseaseAggregator: $(OBJS)
	$(CC) -g $(OBJS) -o $(OUT)

main_aggregator.o: main_aggregator.c
	$(CC) $(FLAGS) main_aggregator.c

main_worker.o: main_worker.c
	$(CC) $(FLAGS) main_worker.c

functions_aggregator.o: functions_aggregator.c
	$(CC) $(FLAGS) functions_aggregator.c

functions_worker.o: functions_worker.c
	$(CC) $(FLAGS) functions_worker.c

extio.o: extio.c
	$(CC) $(FLAGS) extio.c

quicksort_top.o: quicksort_top.c
	$(CC) $(FLAGS) quicksort_top.c

quicksort.o: quicksort.c
	$(CC) $(FLAGS) quicksort.c

red_black_tree.o: red_black_tree.c
	$(CC) $(FLAGS) red_black_tree.c 

listlist.o: listlist.c
	$(CC) $(FLAGS) listlist.c 

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