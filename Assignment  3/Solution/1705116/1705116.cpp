
#include <bits/stdc++.h>
#include<iostream>
#include<cstdio>
#include<cstdlib>
#include<time.h>
#include<unistd.h>
#include<pthread.h>
#include<semaphore.h>

#include <algorithm>
#include <chrono>
#include <fstream>

#include <random>

using namespace std;

using namespace std::chrono;

using std::cout;
using std::cerr;
using std::endl;
using std::string;
using std::ifstream;


#define PASSENGER_NUMBER 5
#define GLOVAR_NUMBER 3
#define ADDITIONAL_LOCKS 3
#define time_special_kiosk 20

#define mean_value 4.1

int NUMBER_OF_KIOSK;
int NUMBER_OF_BELT;
int PASSENGER_CAPACITY_PER_BELT;

int w_kiosk_check_time;
int x_security_check_time;  //belt
int y_boarding_gate_check_time;
int z_vip_channel_walking_time;

pthread_mutex_t boarding_gate,vip_channel,special_kiosk,global_variables[GLOVAR_NUMBER],locks[ADDITIONAL_LOCKS];
sem_t *belts;
pthread_mutex_t *kiosks;



int needed_time;
int airport_arrival_time = 0;
int ranValue;

int *vip_id = new int[ranValue];
int time_interval[PASSENGER_NUMBER];


pthread_mutex_t kiosk_to_board_enter,vip_waiting,movement,kiosk_to_board_leave,board_to_kiosk_enter,board_to_kiosk_leave;

int kiosk_to_board_count = 0;
int board_to_kiosk_count = 0;

int current_time[PASSENGER_NUMBER+1];

void vip_channel_kiosk_to_board(int pid)
{

    int id = pid;
    printf("Passenger %d has arrived at VIP channel at time %d\n",pid,current_time[pid]);
    fflush(stdout);

    pthread_mutex_lock(&kiosk_to_board_enter);
    kiosk_to_board_count++;
    if(kiosk_to_board_count == 1)
    {

        auto start = high_resolution_clock::now();

        pthread_mutex_lock(&vip_waiting);

        auto stop = high_resolution_clock::now();
        auto duration = duration_cast<microseconds>(stop - start);
        int delay_time=duration.count()/1000;
        current_time[id]+=delay_time;

        printf("Passenger %d (VIP-1st) has entered on the VIP channel at time %d [waiting to get free vip channel %d]\n",pid,current_time[id],delay_time);
        fflush(stdout);

        pthread_mutex_lock(&movement);
    }
    pthread_mutex_unlock(&kiosk_to_board_enter);


    printf("Passenger %d (VIP) has started walking on the VIP channel at time %d\n",pid,current_time[id]);
    fflush(stdout);

    usleep(z_vip_channel_walking_time*1000);
    current_time[id]+=z_vip_channel_walking_time;

    pthread_mutex_lock(&kiosk_to_board_leave);

    kiosk_to_board_count--;
    if(kiosk_to_board_count == 0)
    {
        pthread_mutex_unlock(&vip_waiting);
        pthread_mutex_unlock(&movement);
    }

    pthread_mutex_unlock(&kiosk_to_board_leave);


}

void vip_channel_board_to_kiosk(int pid)
{
    int id = pid;

    printf("Passenger %d (lost pass) have arrived at VIP channel to go to special kiosk through vip channel at time %d\n",pid,current_time[pid]);
    fflush(stdout);

    pthread_mutex_lock(&board_to_kiosk_enter);

    auto start = high_resolution_clock::now();
    pthread_mutex_lock(&vip_waiting);
    auto stop = high_resolution_clock::now();

    auto duration = duration_cast<microseconds>(stop - start);
    int delay_time=duration.count()/1000;
    current_time[id]+=delay_time;


    printf("Passenger %d (lost pass) has entered and walking on the VIP channel at time %d [waiting to get free vip channel %d]\n",pid,current_time[id],delay_time);
    fflush(stdout);

    board_to_kiosk_count++;
    if(board_to_kiosk_count == 1)
    {
        pthread_mutex_lock(&movement);
    }
    pthread_mutex_unlock(&vip_waiting);
    pthread_mutex_unlock(&board_to_kiosk_enter);


    usleep(z_vip_channel_walking_time*1000);
    current_time[id]+=z_vip_channel_walking_time;

    pthread_mutex_lock(&board_to_kiosk_leave);

    board_to_kiosk_count--;
    if(board_to_kiosk_count == 0)
    {
        pthread_mutex_unlock(&movement);
    }

    pthread_mutex_unlock(&board_to_kiosk_leave);

    printf("Passenger %d (lost pass) have crossed the VIP channel and reached at special kiosk at time %d\n",pid,current_time[id]);
    fflush(stdout);

}


void* air_journey_system(void* arg)
{
    int id = *((int*)arg);
    delete (int*)arg;
    arg = NULL;


    // kiosk part
    int which_kiosk =(rand()%(NUMBER_OF_KIOSK));

    auto start = high_resolution_clock::now();
    pthread_mutex_lock(&kiosks[which_kiosk]); // unavailable nth kiosk
    auto stop = high_resolution_clock::now();

    auto duration = duration_cast<microseconds>(stop - start);
    int delay_time=duration.count()/1000;


   
    current_time[id]+=delay_time+time_interval[id-1];

    printf("Passenger %d has started self-check in at kiosk %d at time %d [waiting %d ]\n", id,which_kiosk+1,current_time[id],delay_time);
    fflush(stdout);

    usleep(w_kiosk_check_time*1000);
    current_time[id]+=w_kiosk_check_time;


    printf("Passenger %d has finished check in at time %d\n", id,current_time[id]);
    fflush(stdout);

    pthread_mutex_unlock(&kiosks[which_kiosk]);

    // belt part
 

    bool is_vip = false;
    for(int i=0; i<ranValue; i++)
    {
        if(id == vip_id[i])
        {
            is_vip = true;
            break;
        }

    }

    if(is_vip)
    {
        printf("Passenger %d (VIP) has skiped the security check at time %d\n",id,current_time[id]);
        fflush(stdout);
        vip_channel_kiosk_to_board(id);
        printf("Passenger %d (VIP) has crossed  the VIP channel and reached on boarding gate at time %d\n",id,current_time[id]);
        fflush(stdout);


    }
    else
    {
        int which_belt =(rand()%(NUMBER_OF_BELT));
        printf("Passenger %d has started  waiting for the security check in belt %d from time %d\n",id,which_belt+1,current_time[id]);
        fflush(stdout);
        auto start1 = high_resolution_clock::now();

        sem_wait(&belts[which_belt]);
        auto stop1 = high_resolution_clock::now();


        auto duration1 = duration_cast<microseconds>(stop1 - start1);
        int delay_time1 =duration1.count()/1000;
     
        current_time[id]+=delay_time1;

        printf("Passenger %d has started the security check at time %d [waiting %d ]\n",id,current_time[id],delay_time1);
        fflush(stdout);
        usleep(x_security_check_time*1000);

        current_time[id]+= x_security_check_time;

        printf("Passenger %d has crossed the security check at time %d\n", id,current_time[id]);

        fflush(stdout);

        sem_post(&belts[which_belt]);
    }


    // boarding gate

    auto start2 = high_resolution_clock::now();
    pthread_mutex_lock(&boarding_gate);
    auto stop2 = high_resolution_clock::now();

    auto duration2 = duration_cast<microseconds>(stop2 - start2);
    int delay_time2 =duration2.count()/1000;
    current_time[id]+=delay_time2;


    printf("Passenger %d has started boarding the plane at time %d [waiting %d ] \n", id,current_time[id],delay_time2);
    fflush(stdout);

    int p = PASSENGER_NUMBER/2;
    int rn=(rand()%p)+3;
    int lost_check =(id%rn);

    if(!(lost_check))
    {
        pthread_mutex_unlock(&boarding_gate);
        
        // Right to Left

        vip_channel_board_to_kiosk(id);
       
        //Special kiosk
        auto start8 = high_resolution_clock::now();
        pthread_mutex_lock(&special_kiosk);
        auto stop8 = high_resolution_clock::now();

        auto duration8 = duration_cast<microseconds>(stop8 - start8);
        int delay_time8=duration8.count()/1000;

        usleep(w_kiosk_check_time*1000);
        current_time[id]+=w_kiosk_check_time+delay_time8;
        printf("Passenger %d (lost pass) have finished re-pass task to special kiosk  at time %d\n",id,current_time[id]);
        fflush(stdout);
        pthread_mutex_unlock(&special_kiosk);

        //Left to Right back
        vip_channel_kiosk_to_board(id);
        printf("Passenger %d (lost pass) has crossed  the VIP channel and again reached on boarding gate at time %d\n",id,current_time[id]);
        fflush(stdout);


        // re-try in boarding gate
        auto start7 = high_resolution_clock::now();
        pthread_mutex_lock(&boarding_gate);
        auto stop7 = high_resolution_clock::now();

        auto duration7 = duration_cast<microseconds>(stop7 - start7);
        int delay_time7=duration7.count()/1000;


        current_time[id]+=y_boarding_gate_check_time+delay_time7;
        usleep(y_boarding_gate_check_time*1000);
        printf("Passenger %d (lost pass) has boarded the plane at time %d [boarding gate waiting %d]\n",id,current_time[id],delay_time7);
        fflush(stdout);
        pthread_mutex_unlock(&boarding_gate);

    }
    else
    {
        current_time[id]+=y_boarding_gate_check_time;

        usleep(y_boarding_gate_check_time*1000);
        printf("Passenger %d has boarded the plane at time %d\n",id,current_time[id]);
        fflush(stdout);
        pthread_mutex_unlock(&boarding_gate);
    }


}
int main(int argc, char* argv[])
{
    int return_value ;
    srand(time(0));


    string filename("input.txt");
    int number;

    ifstream input_file(filename);
    if (!input_file.is_open())
    {
        cerr << "Could not open the file - '"
             << filename << "'" << endl;
        return EXIT_FAILURE;
    }
    int index=1;
    while (input_file >> number)
    {
        if(index ==1 )
        {
            NUMBER_OF_KIOSK = number;
            index++;
            kiosks = new pthread_mutex_t[NUMBER_OF_KIOSK];
        }
        else if(index == 2)
        {
            NUMBER_OF_BELT = number;
            index++;
            belts = new sem_t[NUMBER_OF_BELT];

        }
        else if(index == 3)
        {
            PASSENGER_CAPACITY_PER_BELT = number;
            index++;

        }
        else if(index == 4)
        {
            w_kiosk_check_time = number;
            index++;

        }
        else if(index == 5)
        {
            x_security_check_time = number;
            index++;

        }
        else if(index == 6)
        {
            y_boarding_gate_check_time = number;
            index++;

        }
        else if(index == 7)
        {
            z_vip_channel_walking_time = number;
            index++;

        }
    }

    input_file.close();




    // initializing semaphore and locks(mutex)


    for(int i=0; i<NUMBER_OF_BELT; i++)
    {

        return_value = sem_init(&belts[i],0,PASSENGER_CAPACITY_PER_BELT);
        if(return_value != 0)
        {
            printf("belts[%d] mutex initialization failed\n",i);
            fflush(stdout);

        }
    }

    return_value = pthread_mutex_init(&boarding_gate,NULL);
    if(return_value != 0)
    {
        printf("boarding_gate  mutex initialization failed\n");
        fflush(stdout);

    }

    return_value = pthread_mutex_init(&vip_channel,NULL);
    if(return_value != 0)
    {
        printf("vip_channel mutex initialization failed\n");
        fflush(stdout);

    }
    return_value = pthread_mutex_init(&special_kiosk,NULL);
    if(return_value != 0)
    {
        printf("special_kiosk mutex initialization failed\n");
        fflush(stdout);
    }

    for(int i=0; i<NUMBER_OF_KIOSK; i++)
    {

        return_value = pthread_mutex_init(&kiosks[i],NULL);
        if(return_value != 0)
        {
            printf("kiosks[%d] mutex initialization failed\n",i);
            fflush(stdout);

        }
    }

    for(int i=0; i<GLOVAR_NUMBER; i++)
    {

        return_value = pthread_mutex_init(&global_variables[i],NULL);
        if(return_value != 0)
        {
            printf("global_variables[%d] mutex initialization failed\n",i);
            fflush(stdout);

        }
    }

    for(int i=0; i<ADDITIONAL_LOCKS; i++)
    {

        return_value = pthread_mutex_init(&locks[i],NULL);
        if(return_value != 0)
        {
            printf("locks[%d] mutex initialization failed\n",i);
            fflush(stdout);

        }
    }

    //creating passengers thread
    int v = PASSENGER_NUMBER/2;
    ranValue = (rand()%(v))+1;


    for(int i=0; i<ranValue; i++)
    {
        vip_id[i]=(rand()%PASSENGER_NUMBER)+1;
        //printf("vip_id[%d] : %d\n",i,vip_id[i]);
        //fflush(stdout);
    }





    const int nrolls = 10000; // number of experiments
    const int nstars = 50000;   // maximum number of stars to distribute

    std::default_random_engine generator;
    std::poisson_distribution<int> distribution(mean_value);

    int p[PASSENGER_NUMBER]= {};

    for (int i=0; i<nrolls; ++i)
    {
        int number = distribution(generator);
        if (number<PASSENGER_NUMBER)
            ++p[number];
    }

    for(int i=0; i<PASSENGER_NUMBER; i++)
    {

        time_interval[i]=(p[i]*nstars)/nrolls;
        time_interval[i]=(time_interval[i]/1000)+1;

    }


    for(int i=0; i<PASSENGER_NUMBER+1; i++)
    {
        current_time[i]=0;
    }

    pthread_t passengers[PASSENGER_NUMBER];

    for(int i=0; i<PASSENGER_NUMBER; i++)
    {



        int* id = new int (i+1);
        int t = 1000*(time_interval[i]);
        usleep(t);
        return_value = pthread_create(&passengers[i],NULL,air_journey_system,(void*)id);
        printf("Passenger %d has arrived at the airport at time %d\n",i+1,time_interval[i]);
        fflush(stdout);

        if(return_value != 0)
        {
            printf("passengers[%d] thread creation failed\n",i);
            fflush(stdout);

        }
    }

    //waiting for passengers threads to finish

    for(int i=0; i<PASSENGER_NUMBER; i++)
    {



        return_value = pthread_join(passengers[i],NULL);
        if(return_value != 0)
        {
            printf("passengers[%d] thread join failed\n",i);
            fflush(stdout);

        }
    }


    //uninitalizing semaphore and mutex

    for(int i=0; i<NUMBER_OF_BELT; i++)
    {
        return_value = sem_destroy(&belts[i]);
        if(return_value != 0)
        {
            printf("belts[%d] sem uninitialization failed\n", i);
            fflush(stdout);
        }
    }

    return_value = pthread_mutex_destroy(&boarding_gate);
    if(return_value != 0)
    {
        printf("boarding_gate mutex uninitialization failed\n");
        fflush(stdout);
    }

    for(int i=0; i<NUMBER_OF_KIOSK; i++)
    {
        return_value = pthread_mutex_destroy(&kiosks[i]);
        if(return_value != 0)
        {
            printf("kiosks[%d] mutex uninitialization failed\n", i);
            fflush(stdout);
        }
    }

    for(int i=0; i<GLOVAR_NUMBER; i++)
    {
        return_value = pthread_mutex_destroy(&global_variables[i]);
        if(return_value != 0)
        {
            printf("global_variables[%d] mutex uninitialization failed\n", i);
            fflush(stdout);
        }
    }

    for(int i=0; i<ADDITIONAL_LOCKS; i++)
    {
        return_value = pthread_mutex_destroy(&locks[i]);
        if(return_value != 0)
        {
            printf("locks[%d] mutex uninitialization failed\n", i);
            fflush(stdout);
        }
    }

    return 0;

}


