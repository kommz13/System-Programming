#ifndef FUNCTIONS_AGGREGATOR_H
#define FUNCTIONS_AGGREGATOR_H

void loaddirAggregator(char * filename, int * total_countries, char *** countries);

void initializeAggregator(char * ip, int port, int w, int b, int total_countries, char **countries);

void createDataStructuresAggregator();

void interactWithUserAggregator();

void cleanUpDataStructuresAggregator();

void interactWithPipesAggregator(int pipe_b);


#endif