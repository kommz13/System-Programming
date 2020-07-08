#ifndef FUNCTIONS_CLIENT_H
#define FUNCTIONS_CLIENT_H

void loadfileClient(char * filename, int * total_countries, char *** countries);

void initializeClient(int w,  int total_countries, char **countries, char * serverIp, int serverPort);

void interactWithUserClient();

void cleanUpDataStructuresClient(int total_commands, char **commands);

void interactWithPipesAggregator(int pipe_b);

// -----------------------------------------------------------------------------

void requestDiseaseFrequencyWithoutCountry(char * cmd, pthread_mutex_t * lock, int fd, int socketb, char * diseasetoken, char * secondtoken, char * thirdtoken);

void requestDiseaseFrequencyWithCountry(char * cmd, pthread_mutex_t * lock, int fd, int socketb, char * diseasetoken, char * secondtoken, char * thirdtoken, char * countrytoken);

void requestPatientRecord(char * cmd, pthread_mutex_t * lock, int fd, int socketb, char * id);

void patientAdmissionsWithoutCountry(char * cmd, pthread_mutex_t * lock, int fd, int socketb, char * diseasetoken, char * secondtoken, char * thirdtoken);

void patientAdmissionsWithCountry(char * cmd, pthread_mutex_t * lock, int fd, int socketb, char * diseasetoken, char * secondtoken, char * thirdtoken, char * countrytoken);

void patientDischargesWithoutCountry(char * cmd, pthread_mutex_t * lock, int fd, int socketb, char * diseasetoken, char * secondtoken, char * thirdtoken);

void patientDischargesWithCountry(char * cmd, pthread_mutex_t * lock, int fd, int socketb, char * diseasetoken, char * secondtoken, char * thirdtoken, char * countrytoken);

void topKAgeRanges(char * cmd, pthread_mutex_t * lock, int fd, int socketb,char * ktoken, char * countrytoken, char * diseasetoken, char * dateA, char * dateB);

int main_worker_from_pthread_create(int *id, int total_commands, char **commands, char * serverIp, int serverPort, int workers);

#endif