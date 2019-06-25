#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#include "vars.h"

int rest_amount;
int booked_seats;

double succesfull = 0.f;
double serial_seats = 0.f;
double full_theater = 0.f;
double credits_failed = 0.f;

int Ncust;
unsigned int seedp;

int* theater;

int free_tel = Ntel;
int free_cash = Ncash;

double waiting_time = 0;
double service_time = 0;

pthread_mutex_t screen_lock;
pthread_mutex_t zoneA_lock;
pthread_mutex_t zoneB_lock;
pthread_mutex_t zoneC_lock;
pthread_mutex_t tel_lock;
pthread_mutex_t book_lock;
pthread_mutex_t cash_lock;
pthread_mutex_t ammount_lock;

pthread_mutex_t successfull_lock;
pthread_mutex_t serial_seats_lock;
pthread_mutex_t full_theater_lock;
pthread_mutex_t credits_failed_lock;

pthread_mutex_t w_time_lock;
pthread_mutex_t s_time_lock;

pthread_cond_t condtel = PTHREAD_COND_INITIALIZER;
pthread_cond_t condcash = PTHREAD_COND_INITIALIZER;

struct timespec w_start, w_stop;
struct timespec s_start, s_stop;

void *find_ticket(void* threadId) {
    int* tid = (int*) threadId;
    int* seats;
    int rc,num_of_tickets,j,counter,money = 0;
    unsigned int sec;
    double chance,zone;
    
    rc = clock_gettime(CLOCK_REALTIME,&w_start);

    rc = clock_gettime(CLOCK_REALTIME,&s_start);


    rc = pthread_mutex_lock(&tel_lock);
    while (free_tel == 0) {
        rc = pthread_cond_wait(&condtel,&tel_lock);
    }
    free_tel--;
    rc = pthread_mutex_unlock(&tel_lock);

    
    //calcs the number of tickets
    num_of_tickets = rand_r(&seedp)%(Nseathigh - Nseatlow + 1) + Nseatlow;
    seats = (int*) malloc(num_of_tickets*sizeof(int));

    //sleep
    sec = rand_r(&seedp)%(tseathigh - tseatlow + 1) + tseatlow;
    sleep(sec);

    if (booked_seats ==  Nseat*(NzoneA+NzoneB+NzoneC)) {
	rc = pthread_mutex_lock(&full_theater_lock);
	full_theater++;
	rc = pthread_mutex_unlock(&full_theater_lock);

        //lock the counter free_tel
        rc = pthread_mutex_lock(&tel_lock);

        rc = pthread_mutex_lock(&screen_lock);
        printf("Client %d can't find seats because the theater is full\n\n",*tid);
        rc = pthread_mutex_unlock(&screen_lock);

        free_tel++;
        rc = pthread_cond_signal(&condtel);

        rc = pthread_mutex_unlock(&tel_lock);
    } else if (num_of_tickets >  Nseat*(NzoneA+NzoneB+NzoneC) - booked_seats) {
        //lock the counter free_tel
        rc = pthread_mutex_lock(&tel_lock);

        rc = pthread_mutex_lock(&screen_lock);
        printf("Client %d wants more seats than the available ones!\n\n",*tid);
        rc = pthread_mutex_unlock(&screen_lock);

        free_tel++;
        rc = pthread_cond_signal(&condtel);

        rc = pthread_mutex_unlock(&tel_lock);
    } else {
        //sleep
        sec = rand_r(&seedp)%(tcashhigh - tcashlow + 1) + tcashlow;
        sleep(sec);

        //find zone
        zone = rand_r(&seedp)%100/100.f;
        if (zone < PzoneA) {
            //zoneA
            rc = pthread_mutex_lock(&zoneA_lock);

            for (int i = 0; i < NzoneA; i++) {
                counter = 0;
                for (j = i*Nseat; j < (i+1)*Nseat; j++) {
                    if (*(theater + j) == 0) {
                    counter++;
                    if (counter == num_of_tickets) break;
                    }
                }
                if (counter == num_of_tickets) {
                    counter = 0;
                    for (j = i*Nseat; j < (i+1)*Nseat; j++) {
                        if (*(theater + j) == 0) {
                    *(seats + counter) = j;
                        counter++;
                        theater[j] = *tid;
                        if (counter == num_of_tickets) break;
                        }
                    }

                    rc = pthread_mutex_lock(&book_lock);
                    booked_seats += num_of_tickets;
                    rc = pthread_mutex_unlock(&book_lock);

                    money = num_of_tickets*CzoneA;

                    break;
                } else {
                    counter = 0;
                }
            }

        rc = pthread_mutex_unlock(&zoneA_lock);
        if (counter == 0) {
	    rc = pthread_mutex_lock(&serial_seats_lock);
	    serial_seats++;
	    rc = pthread_mutex_unlock(&serial_seats_lock);

            rc = pthread_mutex_lock(&screen_lock);
            printf("There are not %d sequence seats at zone A for client %d\n\n",num_of_tickets,*tid);
            rc = pthread_mutex_unlock(&screen_lock);
        }
    } else if (zone < PzoneA + PzoneB) {
        //zoneB
        rc = pthread_mutex_lock(&zoneB_lock);

        for (int i = NzoneA; i < NzoneA + NzoneB; i++) {
            counter = 0;
            for (j = i*Nseat; j < (i+1)*Nseat; j++) {
                if (*(theater + j) == 0) {
                counter++;
                if (counter == num_of_tickets) break;
                }
            }
            if (counter == num_of_tickets) {

                counter = 0;
                for (j = i*Nseat; j < (i+1)*Nseat; j++) {
                    if (*(theater + j) == 0) {
                *(seats + counter) = j;
                    counter++;
                    theater[j] = *tid;
                    if (counter == num_of_tickets) break;
                    }
                }
                
                rc = pthread_mutex_lock(&book_lock);
                booked_seats += num_of_tickets;
                rc = pthread_mutex_unlock(&book_lock);

                money = num_of_tickets*CzoneB;

                break;
            } else {
                counter = 0;
            }
        }

        rc = pthread_mutex_unlock(&zoneB_lock);
        if (counter == 0) {
	    rc = pthread_mutex_lock(&serial_seats_lock);
	    serial_seats++;
	    rc = pthread_mutex_unlock(&serial_seats_lock);

            rc = pthread_mutex_lock(&screen_lock);
            printf("There are not %d sequence seats at zone B for client %d\n\n",num_of_tickets,*tid);
            rc = pthread_mutex_unlock(&screen_lock);
        }
    } else {
        //zoneC
        rc = pthread_mutex_lock(&zoneC_lock);

        for (int i = NzoneA + NzoneB; i < NzoneA + NzoneB + NzoneC; i++) {
            counter = 0;
            for (j = i*Nseat; j < (i+1)*Nseat; j++) {
                if (*(theater + j) == 0) {
                counter++;
                if (counter == num_of_tickets) break;
                }
            }
            if (counter == num_of_tickets) {

                counter = 0;
                for (j = i*Nseat; j < (i+1)*Nseat; j++) {
                    if (*(theater + j) == 0) {
                *(seats + counter) = j;
                    counter++;
                    theater[j] = *tid;
                    if (counter == num_of_tickets) break;
                    }
                }
                
                rc = pthread_mutex_lock(&book_lock);
                booked_seats += num_of_tickets;
                rc = pthread_mutex_unlock(&book_lock);

                money = num_of_tickets*CzoneC;

                break;
            } else {
                counter = 0;
            }
        }

        rc = pthread_mutex_unlock(&zoneC_lock);
        if (counter == 0) {
	    rc = pthread_mutex_lock(&serial_seats_lock);
	    serial_seats++;
	    rc = pthread_mutex_unlock(&serial_seats_lock);

            rc = pthread_mutex_lock(&screen_lock);
            printf("There are not %d sequence seats at zone C for client %d\n\n",num_of_tickets,*tid);
            rc = pthread_mutex_unlock(&screen_lock);
        }
    }

    rc = clock_gettime(CLOCK_REALTIME,&w_stop);
    //calc the waiting time
    rc = pthread_mutex_lock(&w_time_lock);
    waiting_time += (w_stop.tv_sec - w_start.tv_sec);
    rc = pthread_mutex_unlock(&w_time_lock);

    //lock the counter free_tel
    rc = pthread_mutex_lock(&tel_lock);
    if (rc != 0) {  
        printf("ERROR: return code from pthread_mutex_unlock() is %d\n", rc);
        pthread_exit(&rc);
    }
    free_tel++;
    rc = pthread_cond_signal(&condtel);
    if (rc != 0) {  
            printf("ERROR: return code from pthread_mutex_unlock() is %d\n", rc);
            pthread_exit(&rc);
        }
    rc = pthread_mutex_unlock(&tel_lock);
    if (rc != 0) {  
        printf("ERROR: return code from pthread_mutex_unlock() is %d\n", rc);
        pthread_exit(&rc);
    }

    if (money != 0) {
        //find cash
        rc = pthread_mutex_lock(&cash_lock);
        while (free_cash == 0) {
            rc = pthread_cond_wait(&condcash,&cash_lock);
            if (rc != 0) {  
                printf("ERROR: return code from pthread_mutex_unlock() is %d\n", rc);
                pthread_exit(&rc);
            }
        }
        free_cash--;
        rc = pthread_mutex_unlock(&cash_lock);
        
        //payment
        chance = (rand_r(&seedp)%100 + 1)/100.0f;
        if (chance <= 1 - Pcardsucces) {
	    rc = pthread_mutex_lock(&credits_failed_lock);
	    credits_failed++;
	    rc = pthread_mutex_unlock(&credits_failed_lock);

            rc = pthread_mutex_lock(&screen_lock);
            printf("The transaction has failed for the client %d!\n\n",*tid);
            rc = pthread_mutex_unlock(&screen_lock);
            
            j = 0;
            if (zone < PzoneA) {
                rc = pthread_mutex_lock(&zoneA_lock);

                for (int i = 0; i < NzoneA*Nseat; i++) {
                    if (*(theater + i) == *tid) {
                        *(theater + i) = 0;
                        j++;
                        if (j == num_of_tickets) break;
                    }
                }

            rc = pthread_mutex_unlock(&zoneA_lock);
            } else if (zone < PzoneA + PzoneB) {
                rc = pthread_mutex_lock(&zoneB_lock);

                for (int i = NzoneA*Nseat; i < (NzoneA + NzoneB)*Nseat; i++) {
                    if (*(theater + i) == *tid) {
                        *(theater + i) = 0;
                        j++;
                        if (j == num_of_tickets) break;
                    }
                }

                rc = pthread_mutex_unlock(&zoneB_lock);
            } else {
                rc = pthread_mutex_lock(&zoneC_lock);

                for (int i = (NzoneA + NzoneB)*Nseat; i < (NzoneA + NzoneB + NzoneC)*Nseat; i++) {
                    if (*(theater + i) == *tid) {
                        *(theater + i) = 0;
                        j++;
                        if (j == num_of_tickets) break;
                    }
                }

                rc = pthread_mutex_unlock(&zoneC_lock);
            }

            rc = pthread_mutex_lock(&book_lock);
            booked_seats -= num_of_tickets;
            rc = pthread_mutex_unlock(&book_lock);

            //free the cash
            rc = pthread_mutex_lock(&cash_lock);

            free_cash++;
            rc = pthread_cond_signal(&condcash);
            if (rc != 0) {  
                printf("ERROR: return code from pthread_mutex_unlock() is %d\n", rc);
                pthread_exit(&rc);
            }

            rc = pthread_mutex_unlock(&cash_lock);
        } else {
            rc = pthread_mutex_lock(&screen_lock);

            printf("The booking was complete successfully!\nThe number of transaction is %d\n",*tid);
            if (zone < PzoneA) {
                printf("The zone is A and your seats are: ");
            } else if (zone < PzoneA + PzoneB) {
                printf("The zone is B and your seats are: ");
            } else {
                printf("The zone is C and your seats are: ");
            }
            for (int i = 0; i < num_of_tickets; i++) {
                printf("%d ",seats[i]);
            }
            printf("\nThe cost is %d\n\n",money);

            rc = pthread_mutex_unlock(&screen_lock);

            //increase the statistics
            rc = pthread_mutex_lock(&successfull_lock);
            succesfull += 1;
            rc = pthread_mutex_unlock(&successfull_lock);

            rc = pthread_mutex_lock(&ammount_lock);
            if (rc != 0) {  
                printf("ERROR: return code from pthread_mutex_unlock() is %d\n", rc);
                pthread_exit(&rc);
            }
                
            rest_amount += money;
                
            rc = pthread_mutex_unlock(&ammount_lock);
            if (rc != 0) {  
                printf("ERROR: return code from pthread_mutex_unlock() is %d\n", rc);
                pthread_exit(&rc);
            }

            //free the cash
            rc = pthread_mutex_lock(&cash_lock);

            free_cash++;
            rc = pthread_cond_signal(&condcash);
            if (rc != 0) {  
                printf("ERROR: return code from pthread_mutex_unlock() is %d\n", rc);
                pthread_exit(&rc);
            }

            rc = pthread_mutex_unlock(&cash_lock);
        }
        }
    }


    rc = clock_gettime(CLOCK_REALTIME,&s_stop);
    if (rc != 0) {  
        printf("ERROR: return code from clock_gettime() is %d\n", rc);
        pthread_exit(&rc);
    }

    //calc the service time
    rc = pthread_mutex_lock(&s_time_lock);
    if (rc != 0) {  
        printf("ERROR: return code from pthread_mutex_unlock() is %d\n", rc);
        pthread_exit(&rc);
    }

    service_time += (s_stop.tv_sec - s_start.tv_sec);

    rc = pthread_mutex_unlock(&s_time_lock);
    if (rc != 0) {  
        printf("ERROR: return code from pthread_mutex_unlock() is %d\n", rc);
        pthread_exit(&rc);
    }

    if (seats != 0)
	free(seats);
    
    pthread_exit(threadId);
}

int main(int argc, char *argv[]) {
    int rc,Ncust = atoi(argv[1]);
    seedp = (unsigned int) atoi(argv[2]);
    pthread_t threads[Ncust];
    int threadIds[Ncust];
    
    theater = (int*) malloc( Nseat*(NzoneA+NzoneB+NzoneC)*sizeof(int));
    for (int seat = 0; seat < Nseat*(NzoneA+NzoneB+NzoneC); ++seat) {
        *(theater + seat) = 0;
    }
    
    rc = pthread_mutex_init(&screen_lock, NULL);
    if (rc != 0) {  
        printf("ERROR: return code from pthread_mutex_init() is %d\n", rc);
        exit(-1);
    }
    rc = pthread_mutex_init(&zoneA_lock, NULL);
    if (rc != 0) {  
        printf("ERROR: return code from pthread_mutex_init() is %d\n", rc);
        exit(-1);
    }
    rc = pthread_mutex_init(&zoneB_lock, NULL);
    if (rc != 0) {  
        printf("ERROR: return code from pthread_mutex_init() is %d\n", rc);
        exit(-1);
    }
    rc = pthread_mutex_init(&zoneC_lock, NULL);
    if (rc != 0) {  
        printf("ERROR: return code from pthread_mutex_init() is %d\n", rc);
        exit(-1);
    }
    rc = pthread_mutex_init(&tel_lock, NULL);
    if (rc != 0) {  
        printf("ERROR: return code from pthread_mutex_init() is %d\n", rc);
        exit(-1);
    }
    rc = pthread_mutex_init(&book_lock, NULL);
    if (rc != 0) {  
        printf("ERROR: return code from pthread_mutex_init() is %d\n", rc);
        exit(-1);
    }
    rc = pthread_mutex_init(&cash_lock, NULL);
    if (rc != 0) {  
        printf("ERROR: return code from pthread_mutex_init() is %d\n", rc);
        exit(-1);
    }
    rc = pthread_mutex_init(&ammount_lock, NULL);
    if (rc != 0) {  
        printf("ERROR: return code from pthread_mutex_init() is %d\n", rc);
        exit(-1);
    }
    rc = pthread_mutex_init(&successfull_lock, NULL);
    if (rc != 0) {  
        printf("ERROR: return code from pthread_mutex_init() is %d\n", rc);
        exit(-1);
    }
    rc = pthread_mutex_init(&serial_seats_lock, NULL);
    if (rc != 0) {  
        printf("ERROR: return code from pthread_mutex_init() is %d\n", rc);
        exit(-1);
    }
    rc = pthread_mutex_init(&full_theater_lock, NULL);
    if (rc != 0) {  
        printf("ERROR: return code from pthread_mutex_init() is %d\n", rc);
        exit(-1);
    }
    rc = pthread_mutex_init(&credits_failed_lock, NULL);
    if (rc != 0) {  
        printf("ERROR: return code from pthread_mutex_init() is %d\n", rc);
        exit(-1);
    }
    rc = pthread_mutex_init(&w_time_lock, NULL);
    if (rc != 0) {  
        printf("ERROR: return code from pthread_mutex_init() is %d\n", rc);
        exit(-1);
    }
    rc = pthread_mutex_init(&s_time_lock, NULL);
    if (rc != 0) {  
        printf("ERROR: return code from pthread_mutex_init() is %d\n", rc);
        exit(-1);
    }
    
    int i = 0;
    booked_seats = 0;
    rest_amount = 0;

    for (i = 0; i < Ncust; ++i) {
        threadIds[i] = i + 1;
        
        rc = pthread_create(&threads[i], NULL, find_ticket, &threadIds[i]);
        if (rc != 0) {
            printf("ERROR: return code from pthread_create() is %d\n", rc);
            exit(-1);
        }
    }
 
    void *status;
    for (i = 0; i < Ncust; ++i) {
        rc = pthread_join(threads[i], &status);
        
        if (rc != 0) {
            printf("ERROR: return code from pthread_join() is %d\n", rc);
            exit(-1);       
        }
    }
    
    rc = pthread_cond_destroy(&condtel);
    if (rc != 0) {
        printf("ERROR: return code from pthread_cond_destroy() is %d\n", rc);
        exit(-1);
    }
    
    rc = pthread_mutex_destroy(&screen_lock);
    if (rc != 0) {
        printf("ERROR: return code from pthread_mutex_destroy() is %d\n", rc);
        exit(-1);
    }
    rc = pthread_mutex_destroy(&zoneA_lock);
    if (rc != 0) {
        printf("ERROR: return code from pthread_mutex_destroy() is %d\n", rc);
        exit(-1);
    }
    rc = pthread_mutex_destroy(&zoneB_lock);
    if (rc != 0) {
        printf("ERROR: return code from pthread_mutex_destroy() is %d\n", rc);
        exit(-1);
    }
    rc = pthread_mutex_destroy(&zoneC_lock);
    if (rc != 0) {
        printf("ERROR: return code from pthread_mutex_destroy() is %d\n", rc);
        exit(-1);
    }
    rc = pthread_mutex_destroy(&book_lock);
    if (rc != 0) {
        printf("ERROR: return code from pthread_mutex_destroy() is %d\n", rc);
        exit(-1);
    }
    rc = pthread_mutex_destroy(&tel_lock);
    if (rc != 0) {
        printf("ERROR: return code from pthread_mutex_destroy() is %d\n", rc);
        exit(-1);
    }
    rc = pthread_mutex_destroy(&cash_lock);
    if (rc != 0) {
        printf("ERROR: return code from pthread_mutex_destroy() is %d\n", rc);
        exit(-1);
    }
    rc = pthread_mutex_destroy(&ammount_lock);
    if (rc != 0) {
        printf("ERROR: return code from pthread_mutex_destroy() is %d\n", rc);
        exit(-1);
    }
    rc = pthread_mutex_destroy(&successfull_lock);
    if (rc != 0) {
        printf("ERROR: return code from pthread_mutex_destroy() is %d\n", rc);
        exit(-1);
    }
    rc = pthread_mutex_destroy(&serial_seats_lock);
    if (rc != 0) {  
        printf("ERROR: return code from pthread_mutex_init() is %d\n", rc);
        exit(-1);
    }
    rc = pthread_mutex_destroy(&full_theater_lock);
    if (rc != 0) {  
        printf("ERROR: return code from pthread_mutex_init() is %d\n", rc);
        exit(-1);
    }
    rc = pthread_mutex_destroy(&credits_failed_lock);
    if (rc != 0) {  
        printf("ERROR: return code from pthread_mutex_init() is %d\n", rc);
        exit(-1);
    }
    rc = pthread_mutex_destroy(&w_time_lock);
    if (rc != 0) {
        printf("ERROR: return code from pthread_mutex_destroy() is %d\n", rc);
        exit(-1);
    }
    rc = pthread_mutex_destroy(&s_time_lock);
    if (rc != 0) {
        printf("ERROR: return code from pthread_mutex_destroy() is %d\n", rc);
        exit(-1);
    }
    
    printf("Zone A\n");
    for (i = 0; i <  NzoneA*Nseat; ++i) {
        if (*(theater + i) != 0) {
            printf("Seat %d / Client %d\n",i+1,theater[i]);
        }
    }

    printf("Zone B\n");
    for (i = NzoneA*Nseat; i <  (NzoneA + NzoneB)*Nseat; ++i) {
        if (*(theater + i) != 0) {
            printf("Seat %d / Client %d\n",i+1,theater[i]);
        }
    }

    printf("Zone C\n");
    for (i = (NzoneA + NzoneB)*Nseat; i <  (NzoneA + NzoneB + NzoneC)*Nseat; ++i) {
        if (*(theater + i) != 0) {
            printf("Seat %d / Client %d\n",i+1,theater[i]);
        }
    }

    printf("Total income: %d\n",rest_amount);
    printf("The statistics of succesfull booking are: %f\n",succesfull/Ncust);
    printf("The statistics of not finding serial seats are: %f\n",serial_seats/Ncust);
    printf("The statistics of full theater are: %f\n",full_theater/Ncust);
    printf("The statistics of credits failed are: %f\n",credits_failed/Ncust);
    printf("Mean waiting time: %f seconds\n",((double) waiting_time/Ncust)*0.1);
    printf("Mean service time: %f seconds\n",((double) service_time/Ncust)*0.1);
    
    free(theater);
    pthread_exit(NULL);

    return 0;
}
