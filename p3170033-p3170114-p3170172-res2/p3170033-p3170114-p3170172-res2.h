#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define Ntel 8
#define Ncash 4
#define Nseat 10
#define NzoneA 5
#define NzoneB 10
#define NzoneC 10
#define PzoneA 2
#define PzoneB 4
#define PzoneC 4
#define CzoneA 30
#define CzoneB 25
#define CzoneC 20
#define Nseatlow 1
#define Nseathigh 5
#define Tseatlow 5
#define Tseathigh 10
#define Tcashlow 2
#define Tcashhigh 4
#define Pcardsuccess 9
#define BILLION 1000000000L

int balance;
int cost;
int tel_waiting_time;
int cash_waiting_time;

//successful transaction
double st;
//error during payment
double ep;
//full zone
double fz;
//not enough available consecutive seats
double cs;

//mutexes
pthread_mutex_t tel_mutex;
pthread_mutex_t bank_mutex;
pthread_mutex_t seatsA_mutex;
pthread_mutex_t seatsB_mutex;
pthread_mutex_t seatsC_mutex;
pthread_mutex_t screen_mutex;
pthread_mutex_t time_mutex;
pthread_mutex_t cash_mutex;

//time
struct timespec start, stop;

double total_waiting_time;
double total_service_time;

//return seats table
int Return[5];

unsigned int seedp;
int tid;

//theater plan
int SeatsA[NzoneA*Nseat];
int SeatsB[NzoneB*Nseat];
int SeatsC[NzoneC*Nseat];