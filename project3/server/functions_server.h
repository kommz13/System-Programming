#ifndef FUNCTIONS_AGGREGATOR_H
#define FUNCTIONS_AGGREGATOR_H

void initializeServer(int q, int s, int queue_b, int w, int b, int total_countries, char **countries);

void createDataStructuresServer();

void createThreadPool();

void cleanUpThreadPool();

void interactWithClientsAndWorkers();

void cleanUpDataStructuresServer();

void interactWithPipesAggregator(int pipe_b);

int createSocket(char ipbuffer[20], int port);

// -------------------------------------------------------------------------

void requestDiseaseFrequencyWithoutCountry(char * cmd, pthread_mutex_t * lock, int fd, int socketb, char * diseasetoken, char * secondtoken, char * thirdtoken);

void requestDiseaseFrequencyWithCountry(char * cmd, pthread_mutex_t * lock, int fd, int socketb, char * diseasetoken, char * secondtoken, char * thirdtoken, char * countrytoken);

void requestPatientRecord(char * cmd, pthread_mutex_t * lock, int fd, int socketb, char * id);

void patientAdmissionsWithoutCountry(char * cmd, pthread_mutex_t * lock, int fd, int socketb, char * diseasetoken, char * secondtoken, char * thirdtoken);

void patientAdmissionsWithCountry(char * cmd, pthread_mutex_t * lock, int fd, int socketb, char * diseasetoken, char * secondtoken, char * thirdtoken, char * countrytoken);

void patientDischargesWithoutCountry(char * cmd, pthread_mutex_t * lock, int fd, int socketb, char * diseasetoken, char * secondtoken, char * thirdtoken);

void patientDischargesWithCountry(char * cmd, pthread_mutex_t * lock, int fd, int socketb, char * diseasetoken, char * secondtoken, char * thirdtoken, char * countrytoken);

void topKAgeRanges(char * cmd, pthread_mutex_t * lock, int fd, int socketb, char * ktoken, char * countrytoken, char * diseasetoken, char * dateA, char * dateB);
#endif