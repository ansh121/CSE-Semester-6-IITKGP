#include <bits/stdc++.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>  
#include <sys/ipc.h>
#include <sys/shm.h>
#include <semaphore.h>
using namespace std;

#define endl "\n"

struct Job{
	int process_id;
	int producer_number;
	int prior;	// lower number means higher priority
	int comp_time;
	int job_id;

	bool operator<(const Job &rhs) const{
		return prior < rhs.prior;
	}

	void show(){
		cout << "process_id : " << process_id;
		cout << " | producer_number : " << producer_number;
		cout << " | priority : " << prior;
		cout << " | comp_time : " << comp_time;
		cout << " | job_id : " << job_id << endl; 
	}
};

int NP, NC, TotalJobs;

struct job_queue{
	#define JMAX 15
	Job jq[JMAX];
	int jsize;
	job_queue():jsize(0){}
	Job top(){ // gets the highest priority job, and removes it from the job_queue
		assert((this->is_empty()) == false);
		Job ret = jq[0];
		for (int i = 0 ; i < jsize - 1; i++){
			jq[i] = jq[i+1];
		}
		jsize--;
		return ret;
	}
	bool is_empty(){
		return (jsize == 0);
	}
	bool is_full(){
		return (jsize == JMAX);
	}
	void insert_job(Job j){
		assert((this->is_full()) ==  false);
		jq[jsize++] = j;
		sort(jq, jq+jsize); // can be optimised
	}
	#undef JMAX
};

struct shared_data{
	int created_jobs;
	int finished_jobs;
	int total_jobs;
	job_queue jq;
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
	j.process_id = proc_id;
	j.producer_number = prod_num;
	j.prior = 1 + (rand()%10);
	j.comp_time = 1 + (rand()%4);
	j.job_id = 1 + (rand()%100000);
	return j;
}

int main(){
	srand(time(NULL));
	time_t tstart, tend;
	cout << "Enter no of producer processes > ";
	cin >> NP;
	cout << endl;

	cout << "Enter no of consumer process > ";
	cin >> NC ;
	cout << endl;

	cout << "Enter no of jobs > ";
	cin >> TotalJobs;
	cout << endl;

	time(&tstart);

	key_t key = ftok("/dev/random", 'b');
	int shmid = shmget(key, 2048, 0666 | IPC_CREAT);
	shared_data *sd = (shared_data *) shmat(shmid, (void *)0, 0);
	sd->init();
	sd->total_jobs = TotalJobs;

	int prod_num;
	int is_producer = 0; // 1, if its a producer process 
	for (prod_num = 1; prod_num <= NP; prod_num++){
		int pid = fork();
		if (!pid){
			is_producer = 1;
			if (is_producer){
				while ((sd->total_jobs) == -1);
				 // spin
				while ((sd->created_jobs) < (sd->total_jobs)){
					if (((sd->jq).is_full()))
						continue;
					// else, take the producer lock
					sem_wait(&(sd->sem_producer));
					if ((sd->created_jobs) < (sd->total_jobs) and !((sd->jq).is_full())){
						// add sleeping wala code, here
						// sleep for a random time between 0 and 3 seconds
						int rtime = rand() % 4;
						sleep(rtime);
						int job_id = (sd->created_jobs) + 1;
						(sd->created_jobs)++;
						Job j_created = create_job(prod_num, job_id);
						// j_created.show();
						(sd->jq).insert_job(j_created);
						cout << "Producer " << prod_num << " created job " << job_id << endl;
						j_created.show();
						// add sleeping wala code, here
					}
					sem_post(&(sd->sem_producer));
				}
				// cout << "Producer " << prod_num << " ended\n";
				shmdt(sd);
				exit(0);
			}
			break; //  this is essential or otherwise child process will start spawning
		}
		else
			cout << "[producer number = " << prod_num << " , pid = " << pid << " ]\n"; 
	} 
	

	int con_num;
	int is_consumer = 0; // 1, if its a consumer process 
	for (con_num = 1; con_num <= NC; con_num++){
		int pid = fork();
		if (!pid){
			is_consumer = 1;
			if (is_consumer){
				while ((sd->total_jobs) == -1); // spin
				while ((sd->finished_jobs) < (sd->total_jobs)){
					if (((sd->jq).is_empty()))
						continue;
					// else, take the consumer lock
					sem_wait(&(sd->sem_consumer));
					if ((sd->finished_jobs) < (sd->total_jobs) and !((sd->jq).is_empty())){
						// add sleeping wala code, here
						// sleep for a random time between 0 and 3 seconds
						cout << "Consumer process " << con_num << " executing" << endl;
						int rtime = rand() % 4;
						sleep(rtime);
						(sd->finished_jobs)++;
						Job j = (sd->jq).top();
						j.show();
						
						sleep(j.comp_time);// job sleep
						
					}
					sem_post(&(sd->sem_consumer));
				}
				// cout << "Consumer " << con_num << " ended\n";
				shmdt(sd);
				exit(0);
			}
			break; //  this is essential or otherwise child process will start spawning
		}
		else
			cout << "[consumer number = " << con_num << " , pid = " << pid << " ]\n"; 
	} 

	while ((sd->created_jobs) != TotalJobs || (sd->finished_jobs) != TotalJobs); // spin lock on main proc

	time(&tend);
	double time_taken = double(tend - tstart);
	cout << "Time taken to execute : " << time_taken << " s\n";
	shmdt(sd);
	shmctl(shmid, IPC_RMID, NULL);
	exit(0);
}