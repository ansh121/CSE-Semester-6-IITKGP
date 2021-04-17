/*********************************
Assignment No - 3
Names - Anshul Choudhary (17CS10005)
		Ayush Kumar (17CS10007)
*********************************/

#include <bits/stdc++.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>  
#include <sys/ipc.h>
#include <sys/shm.h>
#include <semaphore.h>
using namespace std;

#define endl "\n"
#define JMAX 15

struct Job{
	int p_id, producer_number, priority, compute_time, job_id;

	bool operator<(const Job &rhs) const{
		return priority < rhs.priority;
	}
};

void display(Job *J){
	cout << "p_id : " << J->p_id << " | producer_number : " << J->producer_number << " | priority : " << J->priority << " | compute_time : " << J->compute_time << " | job_id : " << J->job_id << endl; 
}

int NP, NC, NJobs;

struct job_queue{

	Job j_queue[JMAX];
	int jsize;
	job_queue():jsize(0){}

	Job top(){ 
		assert((this->is_empty()) == false);
		Job ret = j_queue[0];
		for (int i = 0 ; i < jsize - 1; i++){
			j_queue[i] = j_queue[i+1];
		}
		jsize--;
		return ret;
	}
	void insert_job(Job j){
		assert((this->is_full()) ==  false);
		j_queue[jsize++] = j;
		sort(j_queue, j_queue+jsize); // can be optimised
	}
	bool is_empty(){
		if(jsize == 0) return true;
		else return false;
	}
	bool is_full(){
		if(jsize == JMAX) return true;
		else return false;
	}
};

struct shared_data{
	int created_jobs;
	int finished_jobs;
	int total_jobs;
	job_queue j_queue;
	sem_t sem_consumer;
	sem_t sem_producer;
	void init(){
		created_jobs = 0;
		finished_jobs = 0;
		total_jobs = -1;
		sem_init(&sem_consumer, 1, 1);
		sem_init(&sem_producer, 1, 1);
	}
};

Job create_job(int prod_num, int proc_id){
	Job j;
	j.p_id = proc_id;
	j.producer_number = prod_num;
	j.priority = (rand()%10) + 1;
	j.compute_time = (rand()%4) + 1;
	j.job_id = (rand()%100000) + 1;
	return j;
}

int main(){
	srand(time(NULL));
	time_t tstart, tend;

	cout << "Enter no of producer processes : ";
	cin >> NP;
	cout << "Enter no of consumer process : ";
	cin >> NC ;
	cout << "Enter no of jobs : ";
	cin >> NJobs;

	time(&tstart);

	key_t key = ftok("/dev/random", 'b');
	int shmid = shmget(key, 2048, 0666 | IPC_CREAT);
	shared_data *sd = (shared_data *) shmat(shmid, (void *)0, 0);
	sd->init();
	sd->total_jobs = NJobs;

	int prod_num;
	for (prod_num = 1; prod_num <= NP; prod_num++){
		int pid = fork();
		if (pid==0){
			{
			while ((sd->total_jobs) == -1);

			while ((sd->created_jobs) < (sd->total_jobs)){
				if ((sd->j_queue).is_full())	continue;
				// else, take the producer lock
				sem_wait(&(sd->sem_producer));
				if ((sd->created_jobs) < (sd->total_jobs) and !((sd->j_queue).is_full())){
					// add sleeping wala code, here
					// sleep for a random time between 0 and 3 seconds
					int tm=(rand() % 4);
					sleep(tm);

					int job_id = (sd->created_jobs)++;
					Job j_created = create_job(prod_num, job_id);

					cout << "Producer " << prod_num << " created job " << job_id << endl;
					display(&j_created);
					cout<<endl;

					(sd->j_queue).insert_job(j_created);
				}
				sem_post(&(sd->sem_producer));
			}
			// cout << "Producer " << prod_num << " ended\n";
			shmdt(sd);
			exit(0);
			}
			break;
		}
		else
			cout << "---> producer number = " << prod_num << " , pid = " << pid << endl; 
	} 
	

	int con_num;
	for (con_num = 1; con_num <= NC; con_num++){
		int pid = fork();
		if (pid==0){
			{
			while ((sd->total_jobs) == -1); // spin
			while ((sd->finished_jobs) < (sd->total_jobs)){
				if (((sd->j_queue).is_empty()))
					continue;
				// else, take the consumer lock
				sem_wait(&(sd->sem_consumer));
				if ((sd->finished_jobs) < (sd->total_jobs) and !((sd->j_queue).is_empty())){
					// add sleeping wala code, here
					// sleep for a random time between 0 and 3 seconds
					cout << "Consumer process " << con_num << " executing" << endl;

					int tm = rand() % 4;
					sleep(tm);

					(sd->finished_jobs)++;
					Job j = (sd->j_queue).top();

					cout << "Consumer process " << con_num << " executed" << endl;
					display(&j);
					cout<<endl;
					
					sleep(j.compute_time);// job sleep
					
				}
				sem_post(&(sd->sem_consumer));
			}
			}
			shmdt(sd);
			exit(0);
			
			break; //  this is essential to stop child spawning
		}
		else
			cout << "---> consumer number = " << con_num << " , pid = " << pid << endl; 
	} 

	while ((sd->created_jobs) != NJobs);
	while ((sd->finished_jobs) != NJobs); 

	time(&tend);
	cout << "Time taken to execute : " << double(tend - tstart) << " s\n";
	shmdt(sd);
	shmctl(shmid, IPC_RMID, NULL);
	exit(0);
}