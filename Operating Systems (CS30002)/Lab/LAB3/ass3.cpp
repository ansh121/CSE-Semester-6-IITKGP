/***************************************************
ASSIGNMENT 2
Group No - 49
Members:    
Vasu Gurram     (17CS30014)
Nilesh Mandal	(17CS10031)
****************************************************/

#include<stdio.h>
#include<sys/ipc.h>
#include<sys/shm.h>
#include<sys/types.h>
#include<string.h>
#include<errno.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include <iostream> 
#include <queue> 
#include <sys/wait.h> 
#include <bits/stdc++.h>
#include <time.h>

#define MAX 100
#define BUF_SIZE 8

using namespace std;

typedef struct job_def{
	pid_t process_id;
	int producer_number;
	int priority;
	int compute_time;
	int job_id;
} job;

struct shmseg {
   int complete;	
   int job_created;
   int job_completed;
   job buf[BUF_SIZE]; 
};

job generateJob(pid_t proc_id)
{
	job J;
	J.process_id = proc_id;
	J.job_id = (rand() + 1) % 100000;
	J.compute_time = (rand() + 1) % 4;
	J.priority = (rand() + 1) % 10;
}

bool compareJob(job a, job b) 
{ 
    return (a.priority > b.priority); 
} 

void insert(job *j, int n , int i, job k)
{
	j[i] = k;
	sort(j,j+n, compareJob);
}

job jobDelete(job *j, int n)
{
	job i,k;
	i.priority = 0;
	k =  j[0];
	j[0]= i;
	sort(j,j+n, compareJob);	
	return k;
}

void display(job j)
{
	cout<<"pid: "<<j.process_id<<endl;
	cout<<"p_no: "<<j.producer_number<<endl;
	cout<<"priority: "<<j.priority<<endl;
	cout<<"Compute Time: "<<j.compute_time<<endl;
	cout<<"Job ID: "<<j.job_id<<endl;
	cout<<endl;
}

int main()
{
	srand(time(0));
	int NP,NC,job_count;
	cout << "Enter number of producers: ";
	cin >> NP;

	cout << "Enter number of consumers: ";
	cin >> NC;

	cout << "Enter total number of jobs: ";
	cin >> job_count;

	struct shmseg* shmt;
	key_t key = ftok("shmfile",65); 

	int shmid = shmget(key,1024,0666|IPC_CREAT); 

	shmt = (struct shmseg*) shmat(shmid,(void*)0,0); 
	for(int k = 0;k < BUF_SIZE;k++)
		(shmt->buf)[k].priority = 0;
	shmt->job_created = 0;
	shmt->job_completed = 0;
	
	shmt->complete = 0;

	pid_t array[NC+1];//CONSUMERS
	pid_t arr[NP+1];//PRODUCERS
	
	int id = fork();
	if(id==0){
		arr[0]= getpid();
		// printf("PID : %d\n",arr[0]);
		int i,j;
		for(i=0;i<NP;i++){
			//if(shmt->job_created == job_count)
			//		break;
			int id1 = fork();	
					if(id1==0)
					{
						while(shmt->job_created < job_count)
						{
							arr[i+1]= getpid();
							// printf("PID : %d\n",arr[i+1]);
							job J;
							J.producer_number = i+1;
							J.process_id = getpid();
							J.job_id = (abs(rand()) + 1) % 100000;
							J.compute_time = (abs(rand()) + 1) % 4;
							J.priority = (abs(rand()) + 1) % 10;

							if(shmt->job_created == job_count)
								break;
							insert(shmt->buf,shmt->job_created - shmt->job_completed,i,J);
							display(J);
							//(shmt->buf)[numtimes] = J;
							shmt->job_created += 1;
							//printf("Writing Process: Shared Memory Write");	
						}
						break;
					}
			}
		}

	else{
		int i,j;
		array[0]= getpid();
		// printf("Cons PID : %d\n",array[0]);
		for(i=0;i<NC;i++){
			int id2 = fork();
			if(id2==0)
			{
				
				array[i+1]= getpid();
				// printf("Cons PID : %d\n",array[i+1]);
				while(shmt->job_created - shmt->job_completed == 0 && shmt->job_created != 0)
					  sleep(1);
				//sleep(3);
				job J = jobDelete(shmt->buf,shmt->job_created - shmt->job_completed);
				//display(J);
				shmt->job_completed += 1;
				sleep(J.compute_time);
				break;
			}
		}
		// printf("yippee!\n");
	}
	if(getpid()!=array[0]){
		// printf("%d waiting!\n",getpid());
		wait(NULL);
	}
return 0;
}