
#ifndef REQUEST_H
#define REQUEST_H

typedef struct Request {
    char cmd[100];
    char firsttoken[100];
    char secondtoken[100];
    char thirdtoken[100];
    char fourthtoken[100];
    char fifthtoken[100];
} Request;

typedef struct Response {
    int number;
    int status;
    char buffer[1000];
} Response;


#endif /* REQUEST_H */

