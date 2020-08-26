#define _POSIX_C_SOURCE 199309L
#include <time.h>
#include <string.h>
#include "p3170033-p3170114-p3170172-res2.h"

//total available seats in zone A
int seatsA = Nseat*NzoneA;
//total available seats in zone B
int seatsB = Nseat*NzoneB;
//total available seats in zone C
int seatsC = Nseat*NzoneC;
//available telephones
int tele = Ntel;
//available cashiers
int cash = Ncash;

pthread_cond_t tele_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t cash_cond = PTHREAD_COND_INITIALIZER;

void *Client(void *a) {

	int *tid = (int *)a;
	
	tel_waiting_time = (rand_r(&seedp)%Tseathigh) + Tseatlow;
	
	int tickets = (rand_r(&seedp)%Nseathigh) + Nseatlow;
	
	int payment_success = ((rand_r(&seedp) % 10)+1);
	
	int zone = ((rand_r(&seedp) % 10)+1);
	
	cash_waiting_time = (rand_r(&seedp)%Tcashhigh) + Tcashlow;
	
	int rc;
	
	//wait till tel is available
	while (tele == 0) {
		rc = pthread_cond_wait(&tele_cond, &tel_mutex);
	}
	
	rc = pthread_mutex_lock(&tel_mutex);
	if (rc != 0) {	
		printf("ERROR in %d: Return code from pthread_mutex_lock(&tel_mutex) is %d\n",*tid, rc);
		pthread_exit(&rc);
	}
	tele--;
	
	//waiting tel simulation
	sleep(tel_waiting_time);
	
	rc = pthread_mutex_unlock(&tel_mutex);
	if (rc != 0) {	
		printf("ERROR in %d: Return code from pthread_mutex_unlock(&tel_mutex) is %d\n",*tid, rc);
		pthread_exit(&rc);
	}
	
	tele++;
	rc = pthread_cond_signal(&tele_cond);
	
	rc = pthread_mutex_lock(&time_mutex);
	if (rc != 0) {	
		printf("ERROR in %d: Return code from pthread_mutex_lock(&time_mutex) is %d\n",*tid, rc);
		pthread_exit(&rc);
	}
	
	total_waiting_time+=tel_waiting_time;
	
	rc = pthread_mutex_unlock(&time_mutex);
	if (rc != 0) {	
		printf("ERROR in %d: Return code from pthread_mutex_unlock(&time_mutex) is %d\n",*tid, rc);
		pthread_exit(&rc);
	}
	
	//initialization of Return
	for(int i=0;i<5;i++) {
		Return[i] = -1;
	}
	
	clock_gettime(CLOCK_REALTIME, &start);

	//zoneA
	if(zone<=PzoneA) {

		//full zone A
		if(seatsA==0) {
			
			rc = pthread_mutex_lock(&screen_mutex);
			if (rc != 0) {	
				printf("ERROR in %d: Return code from pthread_mutex_lock(&screen_mutex) is %d\n",*tid, rc);
				pthread_exit(&rc);
			}
			
			printf("%d: Your reservation was cancelled because zone A is full.\n",*tid);
			
			rc = pthread_mutex_unlock(&screen_mutex);
			if (rc != 0) {	
				printf("ERROR in %d: Return code from pthread_mutex_unlock(&screen_mutex) is %d\n",*tid, rc);
				pthread_exit(&rc);
			}
			
			clock_gettime(CLOCK_REALTIME, &stop);
			
			rc = pthread_mutex_lock(&time_mutex);
			if (rc != 0) {	
				printf("ERROR in %d: Return code from pthread_mutex_lock(&time_mutex) is %d\n",*tid, rc);
				pthread_exit(&rc);
			}

			if (start.tv_nsec > stop.tv_nsec) { // clock underflow 
				total_service_time = total_service_time + ((stop.tv_sec - start.tv_sec)-1) + ((stop.tv_nsec - start.tv_nsec)+BILLION)/BILLION;
			}
			else {
				total_service_time = total_service_time + (stop.tv_sec - start.tv_sec) + (stop.tv_nsec - start.tv_nsec)/BILLION;
			}
			
			rc = pthread_mutex_unlock(&time_mutex);
			if (rc != 0) {	
				printf("ERROR in %d: Return code from pthread_mutex_unlock(&time_mutex) is %d\n",*tid, rc);
				pthread_exit(&rc);
			}
			
			fz++;
			
			pthread_exit(NULL);
		}

		//check for available seats in zone A and reserve them
		rc = pthread_mutex_lock(&seatsA_mutex);
		if (rc != 0) {	
			printf("ERROR in %d: Return code from pthread_mutex_lock(&seatsA_mutex) is %d\n",*tid, rc);
			pthread_exit(&rc);
		}

		//r is for Return[]
		int r = 0;
		int t = tickets;

		//i is for the number of row
		for(int i=0;i<NzoneA;i++) {
			if(t!=0) {
				//j is for seats in a row
				for(int j=0;j<Nseat;j++) {
					if(t!=0) {
						//t==tickets: when it's the first ticket to reserve
						if(SeatsA[j + i*Nseat]==0 && t==tickets) {
							SeatsA[j + i*Nseat]=*tid;
							t--;
							Return[r]=j + i*Nseat;
							r++;
							seatsA--;
						}
						else if(SeatsA[j + i*Nseat]==0){
							//if previous seat is reserved for the same client
							if(SeatsA[(j + i*Nseat)-1]==*tid && t<tickets) {
								SeatsA[j + i*Nseat]=*tid;
								t--;
								Return[r]=j + i*Nseat;
								r++;
								seatsA--;
							}
							//if previous seat is not reserved for the same client
							else {
								//return seats
								for(int x=0;x<r;x++){
									SeatsA[Return[x]]=0;
									seatsA++;
								}
								//re-initialize Return[]
									for(int i=0;i<r;i++) {
										Return[i] = -1;
									}
									r = 0;
								//return tickets to t in order to search the rest of the row for consecutive seats
								t = tickets;
							}
						}
					}
					else break;
				}
			}
			else break;
		}
		rc = pthread_mutex_unlock(&seatsA_mutex);
		if (rc != 0) {	
			printf("ERROR in %d: Return code from pthread_mutex_unlock(&seatsA_mutex) is %d\n",*tid, rc);
			pthread_exit(&rc);
		}

		//lack of available consecutive seats in zone A
		if(t!=0){
			
			rc = pthread_mutex_lock(&seatsA_mutex);
			if (rc != 0) {	
				printf("ERROR in %d: Return code from pthread_mutex_lock(&seatsA_mutex) is %d\n",*tid, rc);
				pthread_exit(&rc);
			}
			
			for(int i =0;i<r;i++){
				SeatsA[Return[i]]=0;
				seatsA++;
			}
			
			rc = pthread_mutex_unlock(&seatsA_mutex);
			if (rc != 0) {	
				printf("ERROR in %d: Return code from pthread_mutex_unlock(&seatsA_mutex) is %d\n",*tid, rc);
				pthread_exit(&rc);
			}
			
			rc = pthread_mutex_lock(&screen_mutex);
			if (rc != 0) {	
				printf("ERROR in %d: Return code from pthread_mutex_lock(&screen_mutex) is %d\n",*tid, rc);
				pthread_exit(&rc);
			}
			
			printf("%d: Your reservation was cancelled due to lack of available consecutive seats in zone A. tick %d\n",*tid,t);
			
			rc = pthread_mutex_unlock(&screen_mutex);
			if (rc != 0) {	
				printf("ERROR in %d: Return code from pthread_mutex_unlock(&screen_mutex) is %d\n",*tid, rc);
				pthread_exit(&rc);
			}
			
			clock_gettime(CLOCK_REALTIME, &stop);
			rc = pthread_mutex_lock(&time_mutex);
			if (rc != 0) {	
				printf("ERROR in %d: Return code from pthread_mutex_lock(&time_mutex) is %d\n",*tid, rc);
				pthread_exit(&rc);
			}
			
			if (start.tv_nsec > stop.tv_nsec) { // clock underflow 
				total_service_time = total_service_time + ((stop.tv_sec - start.tv_sec)-1) + ((stop.tv_nsec - start.tv_nsec)+BILLION)/BILLION;
			}
			else {
				total_service_time = total_service_time + (stop.tv_sec - start.tv_sec) + (stop.tv_nsec - start.tv_nsec)/BILLION;
			}
			
			rc = pthread_mutex_unlock(&time_mutex);
			if (rc != 0) {	
				printf("ERROR in %d: Return code from pthread_mutex_unlock(&time_mutex) is %d\n",*tid, rc);
				pthread_exit(&rc);
			}
			
			cs++;
			
			pthread_exit(NULL);
		}

		//PAYMENT
		
		//wait till cash is available
		while (cash == 0) {
			rc = pthread_cond_wait(&cash_cond, &cash_mutex);
		}
		
		rc = pthread_mutex_lock(&cash_mutex);
		if (rc != 0) {	
			printf("ERROR in %d: Return code from pthread_mutex_lock(&cash_mutex) is %d\n",*tid, rc);
			pthread_exit(&rc);
		}
		cash--;

		//waiting cashier simulation
		sleep(cash_waiting_time);
		
		rc = pthread_mutex_unlock(&cash_mutex);
		if (rc != 0) {	
			printf("ERROR in %d: Return code from pthread_mutex_unlock(&cash_mutex) is %d\n",*tid, rc);
			pthread_exit(&rc);
		}
		
		cash++;
		rc = pthread_cond_signal(&cash_cond);
		
		rc = pthread_mutex_lock(&time_mutex);
		if (rc != 0) {	
			printf("ERROR in %d: Return code from pthread_mutex_lock(&time_mutex) is %d\n",*tid, rc);
			pthread_exit(&rc);
		}
		
		total_waiting_time+=cash_waiting_time;
		
		rc = pthread_mutex_unlock(&time_mutex);
		if (rc != 0) {	
			printf("ERROR in %d: Return code from pthread_mutex_unlock(&time_mutex) is %d\n",*tid, rc);
			pthread_exit(&rc);
		}
		
		if(payment_success <= Pcardsuccess) {
			
			cost = CzoneA*tickets;
			
			rc = pthread_mutex_lock(&screen_mutex);
			if (rc != 0) {	
				printf("ERROR in %d: Return code from pthread_mutex_lock(&screen_mutex) is %d\n",*tid, rc);
				pthread_exit(&rc);
			}
		
			printf("%d: Your reservation is complete. Your transaction number is <%d>, your seats are <",*tid, *tid);
			for(int i=0;i<5;i++) {
				if(Return[i]!=-1) {
					printf(" %d ",Return[i]);
				}
			}
			
			printf("> in zone A and your cost is <%d> euros.\n", cost);
			
			rc = pthread_mutex_unlock(&screen_mutex);
			if (rc != 0) {	
				printf("ERROR in %d: Return code from pthread_mutex_unlock(&screen_mutex) is %d\n",*tid, rc);
				pthread_exit(&rc);
			}
			
			rc = pthread_mutex_lock(&bank_mutex);
			if (rc != 0) {	
				printf("ERROR in %d: Return code from pthread_mutex_lock(&bank_mutex) is %d\n",*tid, rc);
				pthread_exit(&rc);
			}
			
			balance = balance + cost;
			
			rc = pthread_mutex_unlock(&bank_mutex);
			if (rc != 0) {	
				printf("ERROR in %d: Return code from pthread_mutex_unlock(&bank_mutex) is %d\n",*tid, rc);
				pthread_exit(&rc);
			}
		}
		//error during transaction
		else {
			
			rc = pthread_mutex_lock(&screen_mutex);
			if (rc != 0) {	
				printf("ERROR in %d: Return code from pthread_mutex_lock(&screen_mutex) is %d\n",*tid, rc);
				pthread_exit(&rc);
			}
			
			printf("%d: Reservation cancelled. Error during transaction.\n", *tid);
			
			rc = pthread_mutex_unlock(&screen_mutex);
			if (rc != 0) {	
				printf("ERROR in %d: Return code from pthread_mutex_unlock(&screen_mutex) is %d\n",*tid, rc);
				pthread_exit(&rc);
			}
			
			rc = pthread_mutex_lock(&seatsA_mutex);
			if (rc != 0) {	
				printf("ERROR in %d: Return code from pthread_mutex_lock(&seatsA_mutex) is %d\n",*tid, rc);
				pthread_exit(&rc);
			}
			
			for(int i=0;i<r;i++){
				SeatsA[Return[i]]= 0;
				seatsA++;
			}
			
			rc = pthread_mutex_unlock(&seatsA_mutex);
			if (rc != 0) {	
				printf("ERROR in %d: Return code from pthread_mutex_unlock(&seatsA_mutex) is %d\n",*tid, rc);
				pthread_exit(&rc);
			}
			
			ep++;
			
			clock_gettime(CLOCK_REALTIME, &stop);
		
			rc = pthread_mutex_lock(&time_mutex);
			if (rc != 0) {	
				printf("ERROR in %d: Return code from pthread_mutex_lock(&time_mutex) is %d\n",*tid, rc);
				pthread_exit(&rc);
			}
			
			if (start.tv_nsec > stop.tv_nsec) { // clock underflow 
				total_service_time = total_service_time + ((stop.tv_sec - start.tv_sec)-1) + ((stop.tv_nsec - start.tv_nsec)+BILLION)/BILLION;
			}
			else {
				total_service_time = total_service_time + (stop.tv_sec - start.tv_sec) + (stop.tv_nsec - start.tv_nsec)/BILLION;
			}
			
			rc = pthread_mutex_unlock(&time_mutex);
			if (rc != 0) {	
				printf("ERROR in %d: Return code from pthread_mutex_unlock(&time_mutex) is %d\n",*tid, rc);
				pthread_exit(&rc);
			}

			pthread_exit(NULL);
		}
		
		clock_gettime(CLOCK_REALTIME, &stop);
		
		rc = pthread_mutex_lock(&time_mutex);
		if (rc != 0) {	
			printf("ERROR in %d: Return code from pthread_mutex_lock(&time_mutex) is %d\n",*tid, rc);
			pthread_exit(&rc);
		}
		
		if (start.tv_nsec > stop.tv_nsec) { // clock underflow 
			total_service_time = total_service_time + ((stop.tv_sec - start.tv_sec)-1) + ((stop.tv_nsec - start.tv_nsec)+BILLION)/BILLION;
		}
		else {
			total_service_time = total_service_time + (stop.tv_sec - start.tv_sec) + (stop.tv_nsec - start.tv_nsec)/BILLION;
		}
		
		rc = pthread_mutex_unlock(&time_mutex);
		if (rc != 0) {	
			printf("ERROR in %d: Return code from pthread_mutex_unlock(&time_mutex) is %d\n",*tid, rc);
			pthread_exit(&rc);
		}
		
		st++;

		pthread_exit(NULL);
	}
	//zone B
	else if(zone>PzoneA && zone<=PzoneA + PzoneB) {
		
		//full zone B
		if(seatsB==0) {
			
			rc = pthread_mutex_lock(&screen_mutex);
			if (rc != 0) {	
				printf("ERROR in %d: Return code from pthread_mutex_lock(&screen_mutex) is %d\n",*tid, rc);
				pthread_exit(&rc);
			}
			
			printf("%d: Your reservation was cancelled because zone B is full.\n",*tid);
			
			rc = pthread_mutex_unlock(&screen_mutex);
			if (rc != 0) {	
				printf("ERROR in %d: Return code from pthread_mutex_unlock(&screen_mutex) is %d\n",*tid, rc);
				pthread_exit(&rc);
			}
			
			clock_gettime(CLOCK_REALTIME, &stop);
			
			rc = pthread_mutex_lock(&time_mutex);
			if (rc != 0) {	
				printf("ERROR in %d: Return code from pthread_mutex_lock(&time_mutex) is %d\n",*tid, rc);
				pthread_exit(&rc);
			}

			if (start.tv_nsec > stop.tv_nsec) { // clock underflow 
				total_service_time = total_service_time + ((stop.tv_sec - start.tv_sec)-1) + ((stop.tv_nsec - start.tv_nsec)+BILLION)/BILLION;
			}
			else {
				total_service_time = total_service_time + (stop.tv_sec - start.tv_sec) + (stop.tv_nsec - start.tv_nsec)/BILLION;
			}
			
			rc = pthread_mutex_unlock(&time_mutex);
			if (rc != 0) {	
				printf("ERROR in %d: Return code from pthread_mutex_unlock(&time_mutex) is %d\n",*tid, rc);
				pthread_exit(&rc);
			}
			
			fz++;
			
			pthread_exit(NULL);
		}

	//check for available seats in zone B and reserve them
		rc = pthread_mutex_lock(&seatsB_mutex);
		if (rc != 0) {	
			printf("ERROR in %d: Return code from pthread_mutex_lock(&seatsB_mutex) is %d\n",*tid, rc);
			pthread_exit(&rc);
		}

		//r is for Return[]
		int r = 0;
		int t = tickets;

		//i is for the number of row
		for(int i=0;i<NzoneB;i++) {
			if(t!=0) {
				//j is for seats in a row
				for(int j=0;j<Nseat;j++) {
					if(t!=0) {
						//t==tickets: when it's the first ticket to reserve
						if(SeatsB[j + i*Nseat]==0 && t==tickets) {
							SeatsB[j + i*Nseat]=*tid;
							t--;
							Return[r]=j + i*Nseat;
							r++;
							seatsB--;
						}
						else {
							//if previous seat is reserved for the same client
							if(SeatsB[(j + i*Nseat)-1]==*tid && t<tickets) {
								SeatsB[j + i*Nseat]=*tid;
								t--;
								Return[r]=j + i*Nseat;
								r++;
								seatsB--;
							}
							//if previous seat is not reserved for the same client
							else {
								//return seats
								for(int x=0;x<r;x++){
									SeatsB[Return[x]]=0;
									seatsB++;
								}
								//re-initialize Return[]
									for(int i=0;i<r;i++) {
										Return[i] = -1;
									}
									r = 0;
								//return tickets to t in order to search the rest of the row for consecutive seats
								t = tickets;
							}
						}
					}
					else break;
				}
			}
			else break;
		}

		rc = pthread_mutex_unlock(&seatsB_mutex);
		if (rc != 0) {	
			printf("ERROR in %d: Return code from pthread_mutex_unlock(&seatsB_mutex) is %d\n",*tid, rc);
			pthread_exit(&rc);
		}

		//lack of available consecutive seats in zone B
		if(t!=0){
			
			rc = pthread_mutex_lock(&seatsB_mutex);
			if (rc != 0) {	
				printf("ERROR in %d: Return code from pthread_mutex_lock(&seatsB_mutex) is %d\n",*tid, rc);
				pthread_exit(&rc);
			}
			
			for(int i =0;i<r;i++){
				SeatsB[Return[i]]=0;
				seatsB++;
			}
			
			rc = pthread_mutex_unlock(&seatsB_mutex);
			if (rc != 0) {	
				printf("ERROR in %d: Return code from pthread_mutex_unlock(&seatsB_mutex) is %d\n",*tid, rc);
				pthread_exit(&rc);
			}
			
			rc = pthread_mutex_lock(&screen_mutex);
			if (rc != 0) {	
				printf("ERROR in %d: Return code from pthread_mutex_lock(&screen_mutex) is %d\n",*tid, rc);
				pthread_exit(&rc);
			}
			
			printf("%d: Your reservation was cancelled due to lack of available consecutive seats in zone B.\n",*tid);
			
			rc = pthread_mutex_unlock(&screen_mutex);
			if (rc != 0) {	
				printf("ERROR in %d: Return code from pthread_mutex_unlock(&screen_mutex) is %d\n",*tid, rc);
				pthread_exit(&rc);
			}
			
			clock_gettime(CLOCK_REALTIME, &stop);
			rc = pthread_mutex_lock(&time_mutex);
			if (rc != 0) {	
				printf("ERROR in %d: Return code from pthread_mutex_lock(&time_mutex) is %d\n",*tid, rc);
				pthread_exit(&rc);
			}
			
			if (start.tv_nsec > stop.tv_nsec) { // clock underflow 
				total_service_time = total_service_time + ((stop.tv_sec - start.tv_sec)-1) + ((stop.tv_nsec - start.tv_nsec)+BILLION)/BILLION;
			}
			else {
				total_service_time = total_service_time + (stop.tv_sec - start.tv_sec) + (stop.tv_nsec - start.tv_nsec)/BILLION;
			}
			
			rc = pthread_mutex_unlock(&time_mutex);
			if (rc != 0) {	
				printf("ERROR in %d: Return code from pthread_mutex_unlock(&time_mutex) is %d\n",*tid, rc);
				pthread_exit(&rc);
			}
			
			cs++;
			
			pthread_exit(NULL);
		}

		//PAYMENT
		
		//wait till cash is available
		while (cash == 0) {
			rc = pthread_cond_wait(&cash_cond, &cash_mutex);
		}
		
		rc = pthread_mutex_lock(&cash_mutex);
		if (rc != 0) {	
			printf("ERROR in %d: Return code from pthread_mutex_lock(&cash_mutex) is %d\n",*tid, rc);
			pthread_exit(&rc);
		}
		cash--;
		
		//waiting cashier simulation
		sleep(cash_waiting_time);
		
		rc = pthread_mutex_unlock(&cash_mutex);
		if (rc != 0) {	
			printf("ERROR in %d: Return code from pthread_mutex_unlock(&cash_mutex) is %d\n",*tid, rc);
			pthread_exit(&rc);
		}
		
		cash++;
		rc = pthread_cond_signal(&cash_cond);
		
		rc = pthread_mutex_lock(&time_mutex);
		if (rc != 0) {	
			printf("ERROR in %d: Return code from pthread_mutex_lock(&time_mutex) is %d\n",*tid, rc);
			pthread_exit(&rc);
		}
		
		total_waiting_time+=cash_waiting_time;
		
		rc = pthread_mutex_unlock(&time_mutex);
		if (rc != 0) {	
			printf("ERROR in %d: Return code from pthread_mutex_unlock(&time_mutex) is %d\n",*tid, rc);
			pthread_exit(&rc);
		}
		
		if(payment_success <= Pcardsuccess) {
			
			cost = CzoneB*tickets;
			
			rc = pthread_mutex_lock(&screen_mutex);
			if (rc != 0) {	
				printf("ERROR in %d: Return code from pthread_mutex_lock(&screen_mutex) is %d\n",*tid, rc);
				pthread_exit(&rc);
			}
		
			printf("%d: Your reservation is complete. Your transaction number is <%d>, your seats are <",*tid, *tid);
			for(int i=0;i<5;i++) {
				if(Return[i]!=-1) {
					printf(" %d ",Return[i]);
				}
			}
			
			printf("> in zone B and your cost is <%d> euros.\n", cost);
			
			rc = pthread_mutex_unlock(&screen_mutex);
			if (rc != 0) {	
				printf("ERROR in %d: Return code from pthread_mutex_unlock(&screen_mutex) is %d\n",*tid, rc);
				pthread_exit(&rc);
			}

			rc = pthread_mutex_lock(&bank_mutex);
			if (rc != 0) {	
				printf("ERROR in %d: Return code from pthread_mutex_lock(&bank_mutex) is %d\n",*tid, rc);
				pthread_exit(&rc);
			}
			
			balance = balance + cost;
			
			rc = pthread_mutex_unlock(&bank_mutex);
			if (rc != 0) {	
				printf("ERROR in %d: Return code from pthread_mutex_unlock(&bank_mutex) is %d\n",*tid, rc);
				pthread_exit(&rc);
			}
		}
		//error during transaction
		else {
			
			rc = pthread_mutex_lock(&screen_mutex);
			if (rc != 0) {	
				printf("ERROR in %d: Return code from pthread_mutex_lock(&screen_mutex) is %d\n",*tid, rc);
				pthread_exit(&rc);
			}
			
			printf("%d: Reservation cancelled. Error during transaction.\n", *tid);
			
			rc = pthread_mutex_unlock(&screen_mutex);
			if (rc != 0) {	
				printf("ERROR in %d: Return code from pthread_mutex_unlock(&screen_mutex) is %d\n",*tid, rc);
				pthread_exit(&rc);
			}
			
			rc = pthread_mutex_lock(&seatsB_mutex);
			if (rc != 0) {	
				printf("ERROR in %d: Return code from pthread_mutex_lock(&seatsB_mutex) is %d\n",*tid, rc);
				pthread_exit(&rc);
			}
			
			for(int i=0;i<r;i++){
				SeatsB[Return[i]]= 0;
				seatsB++;
			}
			
			rc = pthread_mutex_unlock(&seatsB_mutex);
			if (rc != 0) {	
				printf("ERROR in %d: Return code from pthread_mutex_unlock(&seatsB_mutex) is %d\n",*tid, rc);
				pthread_exit(&rc);
			}
			
			ep++;
			
			clock_gettime(CLOCK_REALTIME, &stop);
		
			rc = pthread_mutex_lock(&time_mutex);
			if (rc != 0) {	
				printf("ERROR in %d: Return code from pthread_mutex_lock(&time_mutex) is %d\n",*tid, rc);
				pthread_exit(&rc);
			}
			
			if (start.tv_nsec > stop.tv_nsec) { // clock underflow 
				total_service_time = total_service_time + ((stop.tv_sec - start.tv_sec)-1) + ((stop.tv_nsec - start.tv_nsec)+BILLION)/BILLION;
			}
			else {
				total_service_time = total_service_time + (stop.tv_sec - start.tv_sec) + (stop.tv_nsec - start.tv_nsec)/BILLION;
			}
			
			rc = pthread_mutex_unlock(&time_mutex);
			if (rc != 0) {	
				printf("ERROR in %d: Return code from pthread_mutex_unlock(&time_mutex) is %d\n",*tid, rc);
				pthread_exit(&rc);
			}

			pthread_exit(NULL);
		}
		
		clock_gettime(CLOCK_REALTIME, &stop);
		
		rc = pthread_mutex_lock(&time_mutex);
		if (rc != 0) {	
			printf("ERROR in %d: Return code from pthread_mutex_lock(&time_mutex) is %d\n",*tid, rc);
			pthread_exit(&rc);
		}
		
		if (start.tv_nsec > stop.tv_nsec) { // clock underflow 
			total_service_time = total_service_time + ((stop.tv_sec - start.tv_sec)-1) + ((stop.tv_nsec - start.tv_nsec)+BILLION)/BILLION;
		}
		else {
			total_service_time = total_service_time + (stop.tv_sec - start.tv_sec) + (stop.tv_nsec - start.tv_nsec)/BILLION;
		}
	
		rc = pthread_mutex_unlock(&time_mutex);
		if (rc != 0) {	
			printf("ERROR in %d: Return code from pthread_mutex_unlock(&time_mutex) is %d\n",*tid, rc);
			pthread_exit(&rc);
		}
		
		st++;

		pthread_exit(NULL);
	}
	//zone C
	else {
		
		//full zone C
		if(seatsC==0) {
			
			rc = pthread_mutex_lock(&screen_mutex);
			if (rc != 0) {	
				printf("ERROR in %d: Return code from pthread_mutex_lock(&screen_mutex) is %d\n",*tid, rc);
				pthread_exit(&rc);
			}
			
			printf("%d: Your reservation was cancelled because zone C is full.\n",*tid);
			
			rc = pthread_mutex_unlock(&screen_mutex);
			if (rc != 0) {	
				printf("ERROR in %d: Return code from pthread_mutex_unlock(&screen_mutex) is %d\n",*tid, rc);
				pthread_exit(&rc);
			}
			
			clock_gettime(CLOCK_REALTIME, &stop);
			
			rc = pthread_mutex_lock(&time_mutex);
			if (rc != 0) {	
				printf("ERROR in %d: Return code from pthread_mutex_lock(&time_mutex) is %d\n",*tid, rc);
				pthread_exit(&rc);
			}

			if (start.tv_nsec > stop.tv_nsec) { // clock underflow 
				total_service_time = total_service_time + ((stop.tv_sec - start.tv_sec)-1) + ((stop.tv_nsec - start.tv_nsec)+BILLION)/BILLION;
			}
			else {
				total_service_time = total_service_time + (stop.tv_sec - start.tv_sec) + (stop.tv_nsec - start.tv_nsec)/BILLION;
			}
			
			rc = pthread_mutex_unlock(&time_mutex);
			if (rc != 0) {	
				printf("ERROR in %d: Return code from pthread_mutex_unlock(&time_mutex) is %d\n",*tid, rc);
				pthread_exit(&rc);
			}
			
			fz++;
			
			pthread_exit(NULL);
		}

	//check for available seats in zone C and reserve them
		rc = pthread_mutex_lock(&seatsC_mutex);
		if (rc != 0) {	
			printf("ERROR in %d: Return code from pthread_mutex_lock(&seatsC_mutex) is %d\n",*tid, rc);
			pthread_exit(&rc);
		}

		//r is for Return[]
		int r = 0;
		int t = tickets;

		//i is for the number of row
		for(int i=0;i<NzoneC;i++) {
			if(t!=0) {
				//j is for seats in a row
				for(int j=0;j<Nseat;j++) {
					if(t!=0) {
						//t==tickets: when it's the first ticket to reserve
						if(SeatsC[j + i*Nseat]==0 && t==tickets) {
							SeatsC[j + i*Nseat]=*tid;
							t--;
							Return[r]=j + i*Nseat;
							r++;
							seatsC--;
						}
						else {
							//if previous seat is reserved for the same client
							if(SeatsC[(j + i*Nseat)-1]==*tid && t<tickets) {
								SeatsC[j + i*Nseat]=*tid;
								t--;
								Return[r]=j + i*Nseat;
								r++;
								seatsC--;
							}
							//if previous seat is not reserved for the same client
							else {
								//return seats
								for(int x=0;x<r;x++){
									SeatsC[Return[x]]=0;
									seatsC++;
								}
								//re-initialize Return[]
									for(int i=0;i<r;i++) {
										Return[i] = -1;
									}
									r = 0;
								//return tickets to t in order to search the rest of the row for consecutive seats
								t = tickets;
							}
						}
					}
					else break;
				}
			}
			else break;
		}

		rc = pthread_mutex_unlock(&seatsC_mutex);
		if (rc != 0) {	
			printf("ERROR in %d: Return code from pthread_mutex_unlock(&seatsC_mutex) is %d\n",*tid, rc);
			pthread_exit(&rc);
		}

		//lack of available consecutive seats in zone C
		if(t!=0){
			
			rc = pthread_mutex_lock(&seatsC_mutex);
			if (rc != 0) {	
				printf("ERROR in %d: Return code from pthread_mutex_lock(&seatsC_mutex) is %d\n",*tid, rc);
				pthread_exit(&rc);
			}
			
			for(int i =0;i<r;i++){
				SeatsC[Return[i]]=0;
				seatsC++;
			}
			
			rc = pthread_mutex_unlock(&seatsC_mutex);
			if (rc != 0) {	
				printf("ERROR in %d: Return code from pthread_mutex_unlock(&seatsC_mutex) is %d\n",*tid, rc);
				pthread_exit(&rc);
			}
		
			rc = pthread_mutex_lock(&screen_mutex);
			if (rc != 0) {	
				printf("ERROR in %d: Return code from pthread_mutex_lock(&screen_mutex) is %d\n",*tid, rc);
				pthread_exit(&rc);
			}
			
			printf("%d: Your reservation was cancelled due to lack of available consecutive seats in zone C.\n",*tid);
			
			rc = pthread_mutex_unlock(&screen_mutex);
			if (rc != 0) {	
				printf("ERROR in %d: Return code from pthread_mutex_unlock(&screen_mutex) is %d\n",*tid, rc);
				pthread_exit(&rc);
			}
			
			clock_gettime(CLOCK_REALTIME, &stop);
			
			rc = pthread_mutex_lock(&time_mutex);
			if (rc != 0) {	
				printf("ERROR in %d: Return code from pthread_mutex_lock(&time_mutex) is %d\n",*tid, rc);
				pthread_exit(&rc);
			}
			
			if (start.tv_nsec > stop.tv_nsec) { // clock underflow 
				total_service_time = total_service_time + ((stop.tv_sec - start.tv_sec)-1) + ((stop.tv_nsec - start.tv_nsec)+BILLION)/BILLION;
			}
			else {
				total_service_time = total_service_time + (stop.tv_sec - start.tv_sec) + (stop.tv_nsec - start.tv_nsec)/BILLION;
			}
			
			rc = pthread_mutex_unlock(&time_mutex);
			if (rc != 0) {	
				printf("ERROR in %d: Return code from pthread_mutex_unlock(&time_mutex) is %d\n",*tid, rc);
				pthread_exit(&rc);
			}
			
			cs++;
			
			pthread_exit(NULL);
		}

		//PAYMENT
		
		//wait till cash is available
		while (cash == 0) {
			rc = pthread_cond_wait(&cash_cond, &cash_mutex);
		}
		
		rc = pthread_mutex_lock(&cash_mutex);
		if (rc != 0) {	
			printf("ERROR in %d: Return code from pthread_mutex_lock(&cash_mutex) is %d\n",*tid, rc);
			pthread_exit(&rc);
		}
		cash--;
		
		//waiting cash simulation
		sleep(cash_waiting_time);
		
		rc = pthread_mutex_unlock(&cash_mutex);
		if (rc != 0) {	
			printf("ERROR in %d: Return code from pthread_mutex_unlock(&cash_mutex) is %d\n",*tid, rc);
			pthread_exit(&rc);
		}
		
		cash++;
		rc = pthread_cond_signal(&cash_cond);
		
		rc = pthread_mutex_lock(&time_mutex);
		if (rc != 0) {	
			printf("ERROR in %d: Return code from pthread_mutex_lock(&time_mutex) is %d\n",*tid, rc);
			pthread_exit(&rc);
		}
		
		total_waiting_time+=cash_waiting_time;
		
		rc = pthread_mutex_unlock(&time_mutex);
		if (rc != 0) {	
			printf("ERROR in %d: Return code from pthread_mutex_unlock(&time_mutex) is %d\n",*tid, rc);
			pthread_exit(&rc);
		}
		
		if(payment_success <= Pcardsuccess) {
			
			cost = CzoneC*tickets;
			
			rc = pthread_mutex_lock(&screen_mutex);
			if (rc != 0) {	
				printf("ERROR in %d: Return code from pthread_mutex_lock(&screen_mutex) is %d\n",*tid, rc);
				pthread_exit(&rc);
			}
		
			printf("%d: Your reservation is complete. Your transaction number is <%d>, your seats are <",*tid, *tid);
			for(int i=0;i<5;i++) {
				if(Return[i]!=-1) {
					printf(" %d ",Return[i]);
				}
			}
			
			printf("> in zone C and your cost is <%d> euros.\n", cost);
			
			rc = pthread_mutex_unlock(&screen_mutex);
			if (rc != 0) {	
				printf("ERROR in %d: Return code from pthread_mutex_unlock(&screen_mutex) is %d\n",*tid, rc);
				pthread_exit(&rc);
			}

			rc = pthread_mutex_lock(&bank_mutex);
			if (rc != 0) {	
				printf("ERROR in %d: Return code from pthread_mutex_lock(&bank_mutex) is %d\n",*tid, rc);
				pthread_exit(&rc);
			}
			
			balance = balance + cost;
			
			rc = pthread_mutex_unlock(&bank_mutex);
			if (rc != 0) {	
				printf("ERROR in %d: Return code from pthread_mutex_unlock(&bank_mutex) is %d\n",*tid, rc);
				pthread_exit(&rc);
			}
		}
		//error during transaction
		else {
			
			rc = pthread_mutex_lock(&screen_mutex);
			if (rc != 0) {	
				printf("ERROR in %d: Return code from pthread_mutex_lock(&screen_mutex) is %d\n",*tid, rc);
				pthread_exit(&rc);
			}
			
			printf("%d: Reservation cancelled. Error during transaction.\n", *tid);
			
			rc = pthread_mutex_unlock(&screen_mutex);
			if (rc != 0) {	
				printf("ERROR in %d: Return code from pthread_mutex_unlock(&screen_mutex) is %d\n",*tid, rc);
				pthread_exit(&rc);
			}
			
			rc = pthread_mutex_lock(&seatsC_mutex);
			if (rc != 0) {	
				printf("ERROR in %d: Return code from pthread_mutex_lock(&seatsC_mutex) is %d\n",*tid, rc);
				pthread_exit(&rc);
			}
			
			for(int i=0;i<r;i++){
				SeatsC[Return[i]]= 0;
				seatsC++;
			}
			
			rc = pthread_mutex_unlock(&seatsC_mutex);
			if (rc != 0) {	
				printf("ERROR in %d: Return code from pthread_mutex_unlock(&seatsC_mutex) is %d\n",*tid, rc);
				pthread_exit(&rc);
			}
			
			ep++;
			
			clock_gettime(CLOCK_REALTIME, &stop);
		
			rc = pthread_mutex_lock(&time_mutex);
			if (rc != 0) {	
				printf("ERROR in %d: Return code from pthread_mutex_lock(&time_mutex) is %d\n",*tid, rc);
				pthread_exit(&rc);
			}
			
			if (start.tv_nsec > stop.tv_nsec) { // clock underflow 
				total_service_time = total_service_time + ((stop.tv_sec - start.tv_sec)-1) + ((stop.tv_nsec - start.tv_nsec)+BILLION)/BILLION;
			}
			else {
				total_service_time = total_service_time + (stop.tv_sec - start.tv_sec) + (stop.tv_nsec - start.tv_nsec)/BILLION;
			}
			
			rc = pthread_mutex_unlock(&time_mutex);
			if (rc != 0) {	
				printf("ERROR in %d: Return code from pthread_mutex_unlock(&time_mutex) is %d\n",*tid, rc);
				pthread_exit(&rc);
			}

			pthread_exit(NULL);	
		}
		
		clock_gettime(CLOCK_REALTIME, &stop);
		
		rc = pthread_mutex_lock(&time_mutex);
		if (rc != 0) {	
			printf("ERROR in %d: Return code from pthread_mutex_lock(&time_mutex) is %d\n",*tid, rc);
			pthread_exit(&rc);
		}
		
		if (start.tv_nsec > stop.tv_nsec) { // clock underflow 
			total_service_time = total_service_time + ((stop.tv_sec - start.tv_sec)-1) + ((stop.tv_nsec - start.tv_nsec)+BILLION)/BILLION;
		}
		else {
			total_service_time = total_service_time + (stop.tv_sec - start.tv_sec) + (stop.tv_nsec - start.tv_nsec)/BILLION;
		}
		
		rc = pthread_mutex_unlock(&time_mutex);
		if (rc != 0) {	
			printf("ERROR in %d: Return code from pthread_mutex_unlock(&time_mutex) is %d\n",*tid, rc);
			pthread_exit(&rc);
		}
		
		st++;

		pthread_exit(NULL);	
	}
}

int main(int argc, char *argv[]) {

	if (argc != 3) {
		printf("Wrong input.\n");
		exit(-1);
	}
	
	int Ncust = atoi(argv[1]);
	
	seedp = atoi(argv[2]);
	
	pthread_t *threads;
	
	threads = (pthread_t*)malloc(Ncust*sizeof(pthread_t));
	if(threads==NULL) {
		printf("Not enough memory.\n");
		return -1;
	}
	
	int rc;
	int countNcust[Ncust];
	int customerCount;
	
	//initialize mutexes
	rc = pthread_mutex_init(&tel_mutex, NULL);
	if (rc != 0) {	
		printf("Error occurred while initializing tel_mutex.");
		exit(rc);
	}
	rc = pthread_mutex_init(&bank_mutex, NULL);
	if (rc != 0) {	
		printf("Error occurred while initializing bank_mutex.");
		exit(rc);
	}
	rc = pthread_mutex_init(&seatsA_mutex, NULL);
	if (rc != 0) {	
		printf("Error occurred while initializing seatsA_mutex.");
		exit(rc);
	}
	rc = pthread_mutex_init(&seatsB_mutex, NULL);
	if (rc != 0) {	
		printf("Error occurred while initializing seatsB_mutex.");
		exit(rc);
	}
	rc = pthread_mutex_init(&seatsC_mutex, NULL);
	if (rc != 0) {	
		printf("Error occurred while initializing seatsC_mutex.");
		exit(rc);
	}
	rc = pthread_mutex_init(&screen_mutex, NULL);
	if (rc != 0) {	
		printf("Error occurred while initializing screen_mutex.");
		exit(rc);
	}
	rc = pthread_mutex_init(&time_mutex, NULL);
	if (rc != 0) {	
		printf("Error occurred while initializing time_mutex.");
		exit(rc);
	}
	rc = pthread_mutex_init(&cash_mutex, NULL);
	if (rc != 0) {	
		printf("Error occurred while initializing cash_mutex.");
		exit(rc);
	}
	
   	for(customerCount = 0; customerCount < Ncust; customerCount++) {
		countNcust[customerCount] = customerCount + 1;
		int *tid = &countNcust[customerCount];
		
    	rc = pthread_create(&threads[customerCount], NULL, Client,tid);

    	if (rc != 0) {
    		printf("ERROR in %d: Return code from pthread_create() is %d\n",*tid, rc);
       		exit(-1);
       	}
    }
		
	void *status;
	int i = 0;
	for (i = 0; i < Ncust; i++) {
		rc = pthread_join(threads[i], &status);
		
		if (rc != 0) {
			printf("ERROR in %d: Return code from pthread_join() is %d\n",tid, rc);
			exit(-1);	
		}
	}
	for(int i=0;i<NzoneA*Nseat;i++) {
		if(SeatsA[i]!=0) printf("Zone A/ Seat %d/ client %d\n", i, SeatsA[i]);
	}
	
	for(int i=0;i<NzoneB*Nseat;i++) {
		if(SeatsB[i]!=0) printf("Zone B/ Seat %d/ client %d\n", i, SeatsB[i]);
	}
	
	for(int i=0;i<NzoneC*Nseat;i++) {
		if(SeatsC[i]!=0) printf("Zone C/ Seat %d/ client %d\n", i, SeatsC[i]);
	}
	
	printf("Total earnings are %d.\n", balance);
	
	printf("The percentage of successful transactions is %f%%.\n",(st/Ncust)*100);
	
	printf("The percentage of transactions that failed during payment is %f%%.\n",(ep/Ncust)*100);
	
	printf("The percentage of transactions that failed due to lack of available consecutive seats is %f%%.\n",(cs/Ncust)*100);
	
	printf("The percentage of transactions that failed because the selected zone was full is %f%%.\n",(fz/Ncust)*100);
	
	printf("Average waiting time is %f seconds.\n",total_waiting_time/Ncust);
	
	printf("Average service time is %f seconds.\n",total_service_time/Ncust);
	
	//destroy all mutexes
	pthread_mutex_destroy(&tel_mutex);
	if (rc != 0) {	
		printf("Error occurred while destroying tel_mutex.");
		exit(rc);
	}
	pthread_mutex_destroy(&bank_mutex);
	if (rc != 0) {	
		printf("Error occurred while destroying bank_mutex.");
		exit(rc);
	}
	pthread_mutex_destroy(&seatsA_mutex);
	if (rc != 0) {	
		printf("Error occurred while destroying seatsA_mutex.");
		exit(rc);
	}
	pthread_mutex_destroy(&seatsB_mutex);
	if (rc != 0) {	
		printf("Error occurred while destroying seatsB_mutex.");
		exit(rc);
	}
	pthread_mutex_destroy(&seatsC_mutex);
	if (rc != 0) {	
		printf("Error occurred while destroying seatsC_mutex.");
		exit(rc);
	}
	pthread_mutex_destroy(&screen_mutex);
	if (rc != 0) {	
		printf("Error occurred while destroying screen_mutex.");
		exit(rc);
	}
	pthread_mutex_destroy(&time_mutex);
	if (rc != 0) {	
		printf("Error occurred while destroying time_mutex.");
		exit(rc);
	}
	
	pthread_mutex_destroy(&cash_mutex);
	if (rc != 0) {	
		printf("Error occurred while destroying cash_mutex.");
		exit(rc);
	}
	
	//destroy conditions
	pthread_cond_destroy(&tele_cond);
	if (rc != 0) {	
		printf("Error occurred while destroying tele_cond.");
		exit(rc);
	}
	
	pthread_cond_destroy(&cash_cond);
	if (rc != 0) {	
		printf("Error occurred while destroying cash_cond.");
		exit(rc);
	}
	
	free(threads);

	return 1;
}
